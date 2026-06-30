//
// Created by Roman Yakimkin on 30.06.2026.
//

#pragma once
#include "libs/bit_array.h"

class BitSieve : public BitArray {
public:
    BitSieve(size_t size) : BitArray(size) {
        std::fill(m_data.begin(), m_data.end(), 0xFFFFFFFFFFFFFFFFULL);

        if (size > 0) reset(0);
        if (size > 1) reset(1);
    }
};