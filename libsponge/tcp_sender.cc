#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    bool zero_window = _window_size == 0;
    if (zero_window) {
        _window_size = 1;
    }

    if (_next_seqno == 0) {
        TCPSegment syn_seg;
        syn_seg.header().syn = true;
        syn_seg.header().seqno = wrap(_next_seqno, _isn);
        _next_seqno++;
        _outstanding_segments.push(syn_seg);
        _bytes_in_flight += syn_seg.length_in_sequence_space();
        _segments_out.push(syn_seg);
    }

    size_t window_remaining;
    if (_window_size > _next_seqno - _window_left_edge) {
        window_remaining = _window_size - (_next_seqno - _window_left_edge);
    } else {
        window_remaining = 0;
    }

    while (!(_stream.buffer_empty()) && window_remaining > 0) {
        TCPSegment seg;

        size_t payload_size = min({window_remaining, _stream.buffer_size(), TCPConfig::MAX_PAYLOAD_SIZE});
        seg.payload() = Buffer(_stream.read(payload_size));
        seg.header().seqno = wrap(_next_seqno, _isn);
        _next_seqno += seg.length_in_sequence_space();
        window_remaining -= seg.length_in_sequence_space();
        _bytes_in_flight += seg.length_in_sequence_space();

        if (_stream.eof() && window_remaining > 0 && !_eof_sent) {
            _eof_sent = true;
            seg.header().fin = true;
            _next_seqno++;
            window_remaining--;
            _bytes_in_flight++;
        }

        _outstanding_segments.push(seg);
        _segments_out.push(seg);
    }

    if (_stream.eof() && window_remaining > 0 && !_eof_sent) {
        _eof_sent = true;
        TCPSegment fin_seg;
        fin_seg.header().fin = true;
        fin_seg.header().seqno = wrap(_next_seqno, _isn);
        _next_seqno++;
        _outstanding_segments.push(fin_seg);
        _bytes_in_flight += fin_seg.length_in_sequence_space();
        _segments_out.push(fin_seg);
    }

    if (!timer_running && !_outstanding_segments.empty()) {
        timer_running = true;
        time_since_last_transmission = 0;
    }

    if (zero_window) {
        _window_size = 0;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t ackno_abs = unwrap(ackno, _isn, _window_left_edge);
    if (ackno_abs > _next_seqno || ackno_abs < _window_left_edge) {
        return;
    }

    bool only_window_update = _window_left_edge == ackno_abs;

    _window_left_edge = ackno_abs;

    _window_size = window_size;

    if (!(_outstanding_segments.empty())) {
        for (uint64_t queue_seqno = unwrap(_outstanding_segments.front().header().seqno, _isn, _window_left_edge);
             !_outstanding_segments.empty() &&
             queue_seqno + _outstanding_segments.front().length_in_sequence_space() <= ackno_abs;) {
            queue_seqno += _outstanding_segments.front().length_in_sequence_space();
            _bytes_in_flight -= _outstanding_segments.front().length_in_sequence_space();
            _outstanding_segments.pop();
        }
    }

    if (_outstanding_segments.empty()) {
        timer_running = false;
    } else {
        timer_running = true;
    }

    if (!only_window_update) {
        time_since_last_transmission = 0;
        retransmission_counter = 0;
    }

    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!timer_running) {
        return;
    }
    time_since_last_transmission += ms_since_last_tick;
    size_t retx_timeout = _initial_retransmission_timeout * (1 << retransmission_counter);
    if (retx_timeout <= time_since_last_transmission) {
        time_since_last_transmission = 0;
        TCPSegment seg = _outstanding_segments.front();
        _segments_out.push(seg);
        if (_window_size > 0) {
            retransmission_counter++;
        }
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return retransmission_counter; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
}
