//
// Created by Roman Yakimkin on 25.06.2026.
//

#pragma once
#include <vector>

namespace model {
    struct FactorizedNumber {
        uint64_t m_numb = 0;
        std::vector<uint64_t> m_factors;
        uint16_t m_thread_numb = -1;
        std::chrono::microseconds m_duration_mks;
    };
} // model