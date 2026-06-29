//
// Created by Roman Yakimkin on 27.06.2026.
//

#pragma once

#include <condition_variable>

namespace workers {
    struct PrimeNumbersGeneratedNotifier {
        std::mutex mx;
        std::condition_variable cv;
        bool is_prime_numbers_prepared = false;
    };
}

