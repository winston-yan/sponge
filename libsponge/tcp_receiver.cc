#include "tcp_receiver.hh"

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const WrappingInt32 seqno = seg.header().seqno;

    /* set the ISN if neccessary */
    if (seg.header().syn) {
        _isn = seqno;
        /* must reset two flags to prevent RECONNECTION */
        _syn_received = true;
        _fin_received = false;
    }

    /* disconnected judge should before FIN flag set
     * in order to prevent data with FIN flag
     */
    if (!_syn_received || (_fin_received && _reassembler.empty()))
        return;
    if (seg.header().fin)
        _fin_received = true;

    /* set the last reassembled index as checkpoint for unwrap function */
    uint64_t last_reasm_idx = _rcv_base_idx - 1;

    /* the seqno of syn happened to be the index of first byte in the payload */
    const uint64_t index = unwrap(seqno, _isn, last_reasm_idx) - (!seg.header().syn);

    /* push payload into the StreamReassembler */
    std::string payload = seg.payload().copy();
    _reassembler.push_substring(payload, index, seg.header().fin);

    /* maintain the receive base index */
    this->_rcv_base_idx = _reassembler.get_receive_base();

    /* maintain the acknowledge number */
    this->_ackno = _rcv_base_idx + 1;

    /* if FIN flag received and all data has been reassembled
     * acknowledge number should +1 for handwave ACK
     */
    if (_fin_received && _reassembler.empty())
        ++_ackno;

    return;
}

std::optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_syn_received)
        return std::nullopt;
    return wrap(_ackno, _isn);
}

size_t TCPReceiver::window_size() const { return _capacity - stream_out().buffer_size(); }
