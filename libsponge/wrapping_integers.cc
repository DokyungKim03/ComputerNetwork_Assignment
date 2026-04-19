#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) { return operator+(isn, static_cast<uint32_t>(n)); }

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint32_t reset_n = static_cast<uint32_t>(operator-(n, isn));

    uint64_t remainder = checkpoint % (1ULL << 32);
    uint64_t checkpoint_base = checkpoint - remainder;

    uint64_t abs_n = static_cast<uint64_t>(reset_n) + checkpoint_base;

    uint64_t abs_n_plus = abs_n + (1ULL << 32);
    uint64_t diff_plus = abs_n_plus > checkpoint ? abs_n_plus - checkpoint : checkpoint - abs_n_plus;

    uint64_t diff = abs_n > checkpoint ? abs_n - checkpoint : checkpoint - abs_n;

    uint64_t diff_minus = diff + 1;
    if (abs_n > (1ULL << 32)) {
        uint64_t abs_n_minus = abs_n - (1ULL << 32);
        diff_minus = abs_n_minus > checkpoint ? abs_n_minus - checkpoint : checkpoint - abs_n_minus;
    }
    if (diff_plus < diff) {
        abs_n += (1ULL << 32);
    } else if (diff_minus < diff) {
        abs_n -= (1ULL << 32);
    }

    return abs_n;
}
