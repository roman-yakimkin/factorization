//
// Created by Roman Yakimkin on 27.06.2026.
//

#pragma once
#include <cstdint>
#include <thread>
#include <algorithm>

namespace helpers {

    uint16_t factorizationThreads() {
        uint16_t total_cores = std::thread::hardware_concurrency();

        if (total_cores == 0) total_cores = 2;

        const uint16_t threads = std::max(1, total_cores - 1);

        return threads;
    }
}

