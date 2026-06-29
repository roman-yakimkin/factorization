//
// Created by Roman Yakimkin on 26.06.2026.
//

#pragma once

#include "defs.h"
#include "notifiers.h"
#include "core/contracts/i_prime_numbers_provider.h"

namespace workers {
    class PrimeNumbersProvider : public core::IPrimeNumbersProvider {
        PrimeNumbersGeneratedNotifier& m_factorizer_notifier;
        uint64_t m_max_val;
        std::vector<uint64_t> m_prime_numbers;
    public:
        explicit PrimeNumbersProvider(uint64_t max_val, PrimeNumbersGeneratedNotifier& notifier)
        : m_factorizer_notifier(notifier), m_max_val(max_val) {};

        std::span<uint64_t> getPrimeNumbers() override;

        void operator()();
    };
} // workers