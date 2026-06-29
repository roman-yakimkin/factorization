//
// Created by Roman Yakimkin on 26.06.2026.
//

#include "bit_sieve.h"

BitSieve::BitSieve(size_t size)
        : m_size(size),
          m_data((size + 63) / 64, 0xFFFFFFFFFFFFFFFFULL) {
    if (size > 0) reset(0);
    if (size > 1) reset(1);
}

bool BitSieve::test(size_t index) {
    return (m_data[index / 64] & (1ULL << (index % 64))) != 0;
}

void BitSieve::set(size_t index) {
    m_data[index / 64] |= (1ULL << (index % 64));
}

void BitSieve::reset(size_t index) {
    m_data[index / 64] &= ~(1ULL << (index % 64));
}

size_t BitSieve::size() const {
    return m_size;
}


