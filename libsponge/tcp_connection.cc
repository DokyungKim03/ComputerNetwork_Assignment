#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    // 세그먼트를 받으면, 마지막으로 세그먼트를 받은 시점에서부터의 시간 간격을 0으로 초기화
    _time_since_last_segment_received = 0;

    // RST 세그먼트를 받으면, TCPConnection을 비활성화하고, 송신 스트림과 수신 스트림 모두에 에러를 설정
    if (seg.header().rst) {
        _active = false;
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        return;
    }

    // 연결하기 전에 SYN이 아닌 세그먼트를 받으면 무시
    if (!_receiver.ackno().has_value() && !seg.header().syn) {
        return;
    }

    // 세그먼트를 받으면, TCPReceiver의 segment_received() 메소드를 호출해서 세그먼트를 TCPReceiver에 전달
    _receiver.segment_received(seg);

    // 세그먼트를 받으면, 해당 세그먼트의 ackno와 window size를 TCPSender의 송신 큐에 있는 세그먼트들에 적용
    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
    }
    _sender.fill_window();

    // 송신 큐가 비어있거나, keep-alive 세그먼트인 경우, 빈 세그먼트를 송신 큐에 넣어서 ACK를 보냄
    bool send_queue_empty =
        seg.length_in_sequence_space() > 0 && _sender.segments_out().empty() && _receiver.ackno().has_value();
    bool keep_alive = _receiver.ackno().has_value() && seg.length_in_sequence_space() == 0 &&
                      seg.header().seqno == _receiver.ackno().value() - 1;

    if (send_queue_empty || keep_alive) {
        _sender.send_empty_segment();
    }

    // 상대 peer에서 먼저 연결을 종료하고, 로컬 peer가 연결을 종료하며 FIN을 보낸 후, 상대 peer의 FIN에 대한 ACK를
    // 받으면 TCPConnection을 비활성화
    if (_receiver.stream_out().eof() && _sender.stream_in().eof() && !_linger_after_streams_finish &&
        _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2 && !_sender.bytes_in_flight()) {
        _active = false;
        return;
    }

    // 상대 peer에서 먼저 종료하고, 아직 로컬 peer가 연결을 종료하지 않은 경우, lingering을 하지 않음
    if (_receiver.stream_out().eof() && !_sender.stream_in().eof()) {
        _linger_after_streams_finish = false;
    }

    control_ack();
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    size_t written = _sender.stream_in().write(data);
    _sender.fill_window();
    control_ack();

    return written;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _time_since_last_segment_received += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);

    // lingering 상태 (로컬 peer가 먼저 종료 후 상대 peer가 종료하며 FIN을 보낸 후, 로컬 peer가 ACK를 보낸 시점)에서,
    // 10 * rt_timeout milliseconds 동안 연결을 유지하다가, 추가적인 세그먼트가 오지 않으면 TCPConnection을 비활성화
    if (_receiver.stream_out().eof() && _time_since_last_segment_received >= 10 * _cfg.rt_timeout &&
        _linger_after_streams_finish)
        _active = false;

    // 재전송 횟수가 최대값을 초과하면 TCPConnection을 비활성화하고, RST 세그먼트를 송신 큐에 넣어서 상대 peer에게
    // 연결이 종료되었음을 알림
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        _active = false;
        TCPSegment rst_seg;
        rst_seg.header().rst = true;
        if (_receiver.ackno().has_value()) {
            rst_seg.header().ack = true;
            rst_seg.header().ackno = _receiver.ackno().value();
        }
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _segments_out.push(rst_seg);
        return;
    }

    control_ack();
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    control_ack();
}

void TCPConnection::connect() {
    _sender.fill_window();
    control_ack();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer

            TCPSegment rst_seg;
            rst_seg.header().rst = true;
            if (_receiver.ackno().has_value()) {
                rst_seg.header().ack = true;
                rst_seg.header().ackno = _receiver.ackno().value();
            }
            _sender.stream_in().set_error();
            _receiver.stream_out().set_error();
            _segments_out.push(rst_seg);
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

// 세그먼트를 송신하기 전, 송신 큐에 있는 세그먼트들에 receiver로 부터 ackno와 window size를 받아와 적용하는 함수
void TCPConnection::control_ack() {
    // receiver에서 window size가 16비트로 표현할 수 있는 최대값보다 크면 그 최대값으로 설정, 아니면 receiver에서 받아온
    // window size를 그대로 사용
    uint16_t window_size = (_receiver.window_size() > std::numeric_limits<uint16_t>::max())
                               ? std::numeric_limits<uint16_t>::max()
                               : static_cast<uint16_t>(_receiver.window_size());

    // TCPSender의 송신 큐에 있는 세그먼트들을 모두 꺼내서, receiver로부터 받아온 ackno와 window size를 적용한 후,
    // TCPConnection의 송신 큐에 넣음
    while (!_sender.segments_out().empty()) {
        TCPSegment seg = _sender.segments_out().front();
        _sender.segments_out().pop();

        // receiver로부터 받아온 ackno가 있어야 ackno를 적용 가능. ackno가 없는 경우는 receiver가 아직 SYN 세그먼트를
        // 받지 못한 경우이므로, ackno를 적용하지 않고 window size만 적용해서 세그먼트를 송신 큐에 넣음.
        if (_receiver.ackno().has_value()) {
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
        }

        seg.header().win = window_size;
        _segments_out.push(seg);
    }
}