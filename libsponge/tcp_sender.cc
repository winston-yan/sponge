#include "tcp_config.hh"
#include "tcp_sender.hh"

#include <random>

// Implementation of a TCP sender

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{std::random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _timer(retx_timeout)
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::send_segment(TCPSegment &seg) {
    TCPHeader &header = seg.header();

    /* set seqno for the segment */
    header.seqno = wrap(_next_seqno, _isn);

    /* add segment into outbound queue */
    _segments_out.push(seg);

    /* add segment with positive seqno into outstanding queue, keep track for possible retransmission */
    _outstandings.push(seg);

    size_t len_seq_space = seg.length_in_sequence_space();
    _bytes_in_flight += len_seq_space;
    _next_seqno += len_seq_space;

    /* may need to start the timer */
    if (!_timer.is_running())
        _timer.restart();

    return;
}

void TCPSender::fill_window() {
    /* first time handshake */
    if (!_syn_sent) {
        TCPSegment seg;
        seg.header().syn = _syn_sent = true;
        send_segment(seg);
        return;
    }

    size_t window_size = (_last_window_size == 0) ? 1 : _last_window_size;
    size_t remain_size{};

    while ((remain_size = window_size - (_next_seqno - _send_base_seqno)) > 0 && !_fin_sent) {
        TCPSegment seg;
        /* payload size should be minimum of three variables */
        size_t payload_size = std::min(remain_size, std::min(_stream.buffer_size(), TCPConfig::MAX_PAYLOAD_SIZE));

        /* read payload size from byte stream */
        seg.payload() = _stream.read(payload_size);

        /* FIN flag should be added if stream is ended and still have seqno space */
        if (_stream.eof() && payload_size < remain_size)
            seg.header().fin = _fin_sent = true;

        /* if no bytes (including syn and fin flags) in the segment, end loop */
        if (seg.length_in_sequence_space() == 0)
            break;

        send_segment(seg);
    }

    return;
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    size_t abs_ackno = unwrap(ackno, _isn, _next_seqno);
    /* invalid ack number */
    if (abs_ackno > _next_seqno) return ;

    /* when to update window size (many BUGS here)
     * IF ack number is valid AND
     * IF window size could provide new segment */
    if (_next_seqno <= abs_ackno + window_size)
        _last_window_size = window_size;

    /* this is the old redulicate ack from receiver */
    if (abs_ackno <= _send_base_seqno) return;

    /* stop the timer: ackno is greater than any other ackno received before */
    if (_timer.is_running())
        _timer.stop();
    _timer.set_rto(_initial_retransmission_timeout);

    size_t first_unacked_seqno = _send_base_seqno;

    /* shrink the outstanding segments window if neccessary */
    while (!_outstandings.empty()) {
        const TCPSegment &seg = _outstandings.front();
        size_t abs_seqno = unwrap(seg.header().seqno, _isn, _next_seqno);
        size_t cnt = seg.length_in_sequence_space();

        /* not all the bytes before the ack number from receiver */
        if (abs_seqno + cnt > abs_ackno)
            break;

        first_unacked_seqno = abs_seqno + cnt;
        _bytes_in_flight -= cnt;
        _outstandings.pop();
    }

    _send_base_seqno = first_unacked_seqno;

    /* if there's any outstanding segments, restart the timer */
    if (!_outstandings.empty())
        _timer.restart();

    /* reset consecutive retransmission counter */
    _retrans_op_counter = 0;

    return;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_timer.timeout(ms_since_last_tick))
        return;

    if (_timer.is_running())
        _timer.stop();

    /* timeout guaranteed after this line */
    /* 1st: RETRANSMISSION */
    _segments_out.push(_outstandings.front());

    /* 2nd: if window_size is NONZERO */
    if (_last_window_size > 0) {
        /* consecutive retrans counter */
        ++_retrans_op_counter;
        /* double the timer's rto */
        _timer.double_rto();
    }

    /* 3rd: RESET the timer and RESTART */
    _timer.restart();
    return;
}

unsigned int TCPSender::consecutive_retransmissions() const { return _retrans_op_counter; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    return;
}

