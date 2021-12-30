#include "tcp_connection.hh"

#include <iostream>
#include <limits>

/* private helper functions */

bool TCPConnection::state_listen() const {
    return _sender.next_seqno_absolute() == 0 && !_receiver.ackno().has_value();
}

bool TCPConnection::state_syn_rcvd() const {
    return _receiver.ackno().has_value() && !_receiver.stream_out().input_ended();
}

bool TCPConnection::state_syn_sent() const {
    const uint64_t seqno = _sender.next_seqno_absolute();
    return _receiver.ackno().has_value() && seqno && seqno == _sender.bytes_in_flight();
}

void TCPConnection::send_assembled_segment() {
    std::queue<TCPSegment> &outbound_queue = _sender.segments_out();

    /* assemble the ackno and receive window into _sender's segments */
    while (!outbound_queue.empty()) {
        TCPSegment &seg = outbound_queue.front();
        TCPHeader &header = seg.header();

        /* When we have ackno from _receiver:
         * any time other than the FIRST handshake
         * set ACK flag AND ackno AND window size
         */
        if (_receiver.ackno().has_value()) {
            header.ackno = _receiver.ackno().value();
            header.ack = true;
            /* larger than numeric limit of win */
            header.win = std::min(_receiver.window_size(), std::numeric_limits<size_t>::max());
        }

        /* only send ONE segment with RST if need to send RST */
        if (_aborting) {
            header.rst = true;
            _aborting = false;
        }

        _segments_out.push(seg);
        outbound_queue.pop();
    }

    examine_clean_disconnection();
    return;
}

void TCPConnection::abort_connection(bool send_rst_segment) {
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _connection_active = false;

    if (!send_rst_segment)
        return;

    /* send the empty segment (with RST flag in send_assembled_segment()) */
    _aborting = true;
    /* only send ONE segment with RST if need to send RST */
    if (_sender.segments_out().empty())
        _sender.send_empty_segment();
    send_assembled_segment();
    return;
}

void TCPConnection::examine_clean_disconnection() {
    if (!_receiver.stream_out().input_ended())
        return;

    ByteStream &outbound = _sender.stream_in();

    /* IF inbound stream ends before outbound stream reaches EOF, set the flag */
    if (!outbound.eof())
        _linger_after_streams_finish = false;

    /* IF inbound stream ends (fully assembled) AND
     * IF outbound stream sending FIN AND
     * IF outbound stream fully acked
     */
    else if (_sender.bytes_in_flight() == 0 &&
             (!_linger_after_streams_finish || _time_since_last_seg_rcvd >= 10 * _cfg.rt_timeout))
        _connection_active = false;
    return;
}

/* "Input" interface for the writer/sender-end */

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

void TCPConnection::connect() {
    /* Call send_segment() function at the beginning of the TCP FSM
     *  1. To fill sender's window
     *  2. To trigger SYN flag to be sent actively (with only one seqno)
     * (Step 1 of 3-way handshake)
     */
    _sender.fill_window();
    send_assembled_segment();
    return;
}

size_t TCPConnection::write(const std::string &data) {
    /* write data to the _sender's input stream */
    size_t written_size = _sender.stream_in().write(data);
    /* send it over TCP if possible */
    _sender.fill_window();
    send_assembled_segment();
    return written_size;
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    /* send FIN segment */
    _sender.fill_window();
    send_assembled_segment();
    return;
}

/* "Output" interface for the reader/receiver-end */

// ByteStream &TCPConnection::inbound_stream() { return _receiver.stream_out(); }

/* Methods for the owner or OS to call */

void TCPConnection::segment_received(const TCPSegment &seg) {
    /* make sure current connection is valid */
    if (!_connection_active)
        return;

    /* reset the timer */
    _time_since_last_seg_rcvd = 0;

    const TCPHeader &header = seg.header();

    /* in LISTEN state, if SYN flag is not set, ignore the segment */
    if (state_listen() && !header.syn)
        return;

    /* in SYN_SENT state, if ACK flag is set with payload, ignore the segment */
    // if (state_syn_sent() && header.ack && seg.payload().size()) return ;

    /* receive RST flag segment, abort connection uncleanly but do not send segment */
    if (header.rst) {
        abort_connection(false);
        return;
    }

    /* give the segment to the receiver */
    _receiver.segment_received(seg);

    /* update sender's ackno and window size IF ACK is set */
    if (header.ack) {
        _sender.ack_received(header.ackno, header.win);
        // _sender.fill_window();
    }

    /* IF received segment is Step 1 of 3-way-handshake (judged before giving segment to receiver)
     * MUST NOT send an empty segment, should invoke connect
     */
    if (header.syn && _sender.next_seqno_absolute() == 0) {
        connect();
        return;
    }

    size_t cnt = seg.length_in_sequence_space();
    /* if the incoming segment occupied any seqno, make sure at least one is sent in reply
     * received segment could be
     * Step 2 of 3-way-handshake OR
     * Step 1 of 4-way-handwave OR
     * Step 2 of 4-way-handwave
     * MUST send an empty segment
     */
    if (cnt && _sender.segments_out().empty())
        _sender.send_empty_segment();

    /* empty segment received, should reply with update ackno and window size */
    if (_receiver.ackno().has_value() && cnt == 0 && header.seqno == _receiver.ackno().value() - 1)
        _sender.send_empty_segment();

    /* flush the outbound queue */
    send_assembled_segment();
    return;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _time_since_last_seg_rcvd += ms_since_last_tick;

    /* Tell the _sender passage of time */
    _sender.tick(ms_since_last_tick);

    /* consecutive retransmission count exceeds the limit */
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        abort_connection(true);
    }

    /* sender could have retransmitted, we need to flush the queue
     * and examine whether can be cleanly shut down connection
     */
    send_assembled_segment();
    return;
}

bool TCPConnection::active() const { return _connection_active; }

// std::queue<TCPSegment> &TCPConnection::segments_out() { return _segments_out; }

/* Accessors for testing */

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_seg_rcvd; }

/* destructor sends a RST if the connection is still open */

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            std::cerr << "Warning: Unclean shutdown of TCPConnection\n";
            /* Need to send a RST segment to the peer */
            abort_connection(true);
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
