#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : reassemble_buffer(capacity)
    , output_last_index(0)
    , _output(capacity)
    , _capacity(capacity)
    , eof_received(false)
    , eof_index(0) {}
//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    buffer_resize();
    size_t buffer_last_index = output_last_index + _capacity - _output.buffer_size();
    if (index < buffer_last_index) {
        if (index >= output_last_index) {
            if (index + data.size() >= buffer_last_index) {
                insert_substring_into_buffer(data.substr(0, buffer_last_index - index), index);
            } else {
                insert_substring_into_buffer(data, index);
            }
        } else {
            if (index + data.size() >= output_last_index) {
                if (index + data.size() >= buffer_last_index) {
                    insert_substring_into_buffer(
                        data.substr(output_last_index - index, buffer_last_index - output_last_index),
                        output_last_index);
                } else {
                    insert_substring_into_buffer(data.substr(output_last_index - index), output_last_index);
                }
            }
        }
    }

    if (eof) {
        eof_received = true;
        eof_index = index + data.size();
    }

    if (!reassemble_buffer.empty() && reassemble_buffer[0].valid) {
        push_buffer_into_output();
    }

    if (eof_received && output_last_index >= eof_index) {
        _output.end_input();
    }
}

void StreamReassembler::insert_substring_into_buffer(const string &data, const size_t index) {
    for (size_t i = index; i < index + data.size(); i++) {
        if (!reassemble_buffer[i - output_last_index].valid) {
            reassemble_buffer[i - output_last_index].data = data[i - index];
            reassemble_buffer[i - output_last_index].valid = true;
        }
    }
}

void StreamReassembler::buffer_resize() {
    if (_output.buffer_size() + reassemble_buffer.size() < _capacity) {
        reassemble_buffer.resize(_capacity - _output.buffer_size());
    }
}

void StreamReassembler::push_buffer_into_output() {
    std::string data_to_push;
    for (size_t i = 0; i < reassemble_buffer.size(); i++) {
        if (reassemble_buffer[i].valid) {
            data_to_push.push_back(reassemble_buffer[i].data);
        } else {
            break;
        }
    }
    _output.write(data_to_push);
    output_last_index += data_to_push.size();
    reassemble_buffer.erase(reassemble_buffer.begin(), reassemble_buffer.begin() + data_to_push.size());
}

size_t StreamReassembler::unassembled_bytes() const {
    size_t count = 0;
    for (size_t i = 0; i < reassemble_buffer.size(); i++) {
        if (reassemble_buffer[i].valid) {
            count++;
        }
    }
    return count;
}

bool StreamReassembler::empty() const { return !unassembled_bytes(); }
