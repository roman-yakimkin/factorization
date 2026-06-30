//
// Created by Roman Yakimkin on 26.06.2026.
//

#pragma once

#include <cstdint>
#include <vector>

class BitArray {
protected:
    size_t m_size;
    std::vector<uint64_t> m_data;
public:
    explicit BitArray(size_t size);

    bool test(size_t index) const;
    void set(size_t index);
    void reset(size_t index);
    size_t size() const;
};
