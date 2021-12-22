#include "tcp_sender.hh"

#include "tcp_config.hh"

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

void TCPSender::fill_window() {
    size_t window_size = (_last_window_size == 0) ? 1 : _last_window_size;

    while (window_size > 0) {
        TCPSegment seg;
        TCPHeader &header = seg.header();

        /* set seqno for the segment */
        header.seqno = wrap(_next_seqno, _isn);

        /* payload size should be minimum of three variables */
        size_t payload_size = 
            std::min(window_size - seg.length_in_sequence_space(), 
            std::min(_stream.buffer_size(), TCPConfig::MAX_PAYLOAD_SIZE));
       
        /* add payload from byte stream */
        seg.payload() = _stream.peek_output(payload_size);
        _stream.pop_output(payload_size);

        /* add segment into outbound queue */
        _segments_out.push(seg);

        /* FIN flag should be judged after payload popped
         * FIN flag cannot be sent with SYN flag
         * because when sending SYN flag, window size is 1
         */
        if (_stream.eof() && _syn_sent)
            header.fin = _fin_sent = true;

        /* SYN flag to start transfroming byte stream to segments */
        if (!_syn_sent) 
            header.syn = _syn_sent = true;

        /* may need to start the timer */
        if (!_timer.is_running()) _timer.restart();

        /* add segment with positive seqno into outstanding queue, keep track for possible retransmission */
        _outstandings.push(seg);

        size_t len_seq_space = seg.length_in_sequence_space();
        _bytes_in_flight += len_seq_space;
        _next_seqno += len_seq_space;
        window_size -= len_seq_space;

        /* there is nothing to be read from the byte stream */
        if (_stream.eof()) break;
    }

    return ;
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    /* this is the old redulicate ack from receiver */
    size_t abs_ackno = unwrap(ackno, _isn, _next_seqno);
    if (abs_ackno <= _send_base_seqno) return ;

    /* stop the timer: ackno is greater than any other ackno received before */
    if (_timer.is_running())_timer.stop();
    _timer.set_rto(_initial_retransmission_timeout);

    size_t first_unacked_seqno = _send_base_seqno;

    /* shrink the outstanding segments window if neccessary */
    while (true) { 
        TCPSegment &seg = _outstandings.front();
        size_t abs_seqno = unwrap(seg.header().seqno, _isn, _next_seqno);
        
        /* not all the bytes before the ack number from receiver */ 
        if (abs_seqno + seg.length_in_sequence_space() > abs_ackno)
            break;

        _outstandings.pop();
        first_unacked_seqno = abs_seqno + seg.length_in_sequence_space();
    }
    _send_base_seqno = first_unacked_seqno;

    /* update window size */
    _last_window_size = window_size;

    /* if there's any unstanding segments, restart the timer */
    if (!_outstandings.empty()) _timer.restart();

    /* reset consecutive retransmission counter */
    _retrans_op_counter = 0;

    return ;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_timer.timeout(ms_since_last_tick)) return ;

    if (_timer.is_running())_timer.stop();

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
    return ;
}

unsigned int TCPSender::consecutive_retransmissions() const { return _retrans_op_counter; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    return ;
}
