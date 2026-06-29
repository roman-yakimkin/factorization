//
// Created by Roman Yakimkin on 26.06.2026.
//

#pragma once
#include "defs.h"
#include "notifiers.h"
#include "core/contracts/i_prime_numbers_provider.h"

namespace workers {
    class Factorizer {
        uint16_t m_thread_numb = 0;
        NumberQueue* m_input_queue = nullptr;
        ResultQueue* m_output_queue = nullptr;
        core::IPrimeNumbersProvider* m_prime_numbers = nullptr;
        PrimeNumbersGeneratedNotifier& m_prime_numbers_generated_notifier;

        std::vector<uint64_t> getPrimeFactors(uint64_t numb) const;
    public:
        explicit Factorizer(uint16_t thread_numb,
                   NumberQueue* input_queue,
                   ResultQueue* output_queue,
                   core::IPrimeNumbersProvider* prime_numbers,
                   PrimeNumbersGeneratedNotifier& notifier
        );

        void operator()();
    };
} // workers