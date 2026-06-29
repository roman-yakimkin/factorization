//
// Created by Roman Yakimkin on 26.06.2026.
//

#pragma once

#include <span>
#include <cstdint>

namespace core {
    class IPrimeNumbersProvider {
    public:
        virtual ~IPrimeNumbersProvider() = default;

        virtual std::span<uint64_t> getPrimeNumbers() = 0;
    };
}
