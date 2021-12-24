#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH
#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <functional>
#include <queue>

//! \brief The retransmission timer part of implementation.
//! Helper class for TCPSender class
class RetransTimer {
  private:
    size_t _retransmission_timeout;
    size_t _ms_since_restart;
    bool _timer_running;

  public:
    RetransTimer(unsigned int init_rto)
        : _retransmission_timeout(init_rto), _ms_since_restart(0), _timer_running(false) {}
    void double_rto() { _retransmission_timeout <<= 1; }
    bool is_running() const { return _timer_running; }
    void stop() { _timer_running = false; }
    void restart() {
        _timer_running = true;
        _ms_since_restart = 0;
    }
    void set_rto(size_t rto) { _retransmission_timeout = rto; }
    bool timeout(size_t interval) {
        _ms_since_restart += interval;
        return _ms_since_restart >= _retransmission_timeout;
    }
};

//! \brief The "sender" part of a TCP implementation.

//! Accepts a ByteStream, divides it up into segments and sends the
//! segments, keeps track of which segments are still in-flight,
//! maintains the Retransmission Timer, and retransmits in-flight
//! segments if the retransmission timer expires.
class TCPSender {
  private:
    //! our initial sequence number, the number for our SYN.
    WrappingInt32 _isn;

    //! outbound queue of segments that the TCPSender wants sent
    std::queue<TCPSegment> _segments_out{};

    //! queue of outstanding segments that already sent but not yet acked
    std::queue<TCPSegment> _outstandings{};

    //! retransmission timer for the connection
    unsigned int _initial_retransmission_timeout;

    //! our retransmission timer
    RetransTimer _timer;

    //! outgoing stream of bytes that have not yet been sent
    ByteStream _stream;

    //! the (absolute) sequence number for the next byte to be sent
    uint64_t _next_seqno{0};

    //! the most important send base seqno
    size_t _send_base_seqno{0};

    //! counter for times of consecutive retransmission operations
    unsigned int _retrans_op_counter{0};

    //! refer to the corresponding Accessor
    size_t _bytes_in_flight{0};

    //! window size from last receiver's ack, initially to be ONE
    uint16_t _last_window_size{1ul};

    //! The flag indicates whether SYN/FIN signal is sent
    bool _syn_sent{};
    bool _fin_sent{};

    //! The flag to enable fill_window() function
    bool _fill_window_enable{true};

    //! Helper member function to send segment & manipulate timer
    void send_segment(TCPSegment &seg);

  public:
    //! Initialize a TCPSender
    TCPSender(const size_t capacity = TCPConfig::DEFAULT_CAPACITY,
              const uint16_t retx_timeout = TCPConfig::TIMEOUT_DFLT,
              const std::optional<WrappingInt32> fixed_isn = {});

    //! \name "Input" interface for the writer
    //!@{
    ByteStream &stream_in() { return _stream; }
    const ByteStream &stream_in() const { return _stream; }
    //!@}

    //! \name Methods that can cause the TCPSender to send a segment
    //!@{

    //! \brief A new acknowledgment was received
    void ack_received(const WrappingInt32 ackno, const uint16_t window_size);

    //! \brief Generate an empty-payload segment (useful for creating empty ACK segments)
    void send_empty_segment();

    //! \brief create and send segments to fill as much of the window as possible
    void fill_window();

    //! \brief Notifies the TCPSender of the passage of time
    void tick(const size_t ms_since_last_tick);
    //!@}

    //! \name Accessors
    //!@{

    //! \brief How many sequence numbers are occupied by segments sent but not yet acknowledged?
    //! \note count is in "sequence space," i.e. SYN and FIN each count for one byte
    //! (see TCPSegment::length_in_sequence_space())
    size_t bytes_in_flight() const;

    //! \brief Number of consecutive retransmissions that have occurred in a row
    unsigned int consecutive_retransmissions() const;

    //! \brief TCPSegments that the TCPSender has enqueued for transmission.
    //! \note These must be dequeued and sent by the TCPConnection,
    //! which will need to fill in the fields that are set by the TCPReceiver
    //! (ackno and window size) before sending.
    std::queue<TCPSegment> &segments_out() { return _segments_out; }
    //!@}

    //! \name What is the next sequence number? (used for testing)
    //!@{

    //! \brief absolute seqno for the next byte to be sent
    uint64_t next_seqno_absolute() const { return _next_seqno; }

    //! \brief relative seqno for the next byte to be sent
    WrappingInt32 next_seqno() const { return wrap(_next_seqno, _isn); }
    //!@}
};

#endif  // SPONGE_LIBSPONGE_TCP_SENDER_HH
