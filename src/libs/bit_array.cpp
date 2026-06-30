//
// Created by Roman Yakimkin on 26.06.2026.
//

#include "bit_array.h"

BitArray::BitArray(size_t size) : m_size(size), m_data((size + 63) / 64) {}

bool BitArray::test(size_t index) const {
    return (m_data[index / 64] & 1ULL << (index % 64)) != 0;
}

void BitArray::set(size_t index) {
    m_data[index / 64] |= (1ULL << (index % 64));
}

void BitArray::reset(size_t index) {
    m_data[index / 64] &= ~(1ULL << (index % 64));
}

size_t BitArray::size() const {
    return m_size;
}


