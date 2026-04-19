#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    bool syn_flag = seg.header().syn;
    bool fin_flag = seg.header().fin;
    WrappingInt32 seqno = seg.header().seqno;
    uint64_t abs_seqno;

    if (syn_flag && !_syn_received) {
        _isn = seqno.raw_value();
        _syn_received = true;
    }

    if (_syn_received) {
        if (syn_flag)
            abs_seqno = 0;
        else
            abs_seqno = unwrap(seqno, WrappingInt32(_isn), _reassembler.stream_out().bytes_written()) - 1;
        _reassembler.push_substring(seg.payload().copy(), abs_seqno, fin_flag);
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_syn_received) {
        return WrappingInt32(
            wrap(_reassembler.stream_out().bytes_written() + (_reassembler.stream_out().input_ended() ? 2 : 1),
                 WrappingInt32(_isn)));
    } else {
        return nullopt;
    }
}

size_t TCPReceiver::window_size() const { return {_capacity - _reassembler.stream_out().buffer_size()}; }
