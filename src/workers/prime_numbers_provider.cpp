//
// Created by Roman Yakimkin on 26.06.2026.
//

#include "prime_numbers_provider.h"

#include "libs/bit_sieve.h"

namespace workers {
    std::span<uint64_t> PrimeNumbersProvider::getPrimeNumbers() {
        return m_prime_numbers;
    }

    void PrimeNumbersProvider::operator()() {
        std::cout << "Prime numbers provider thread started.\n";

        BitSieve sieve(m_max_val + 1);

        for (uint64_t i = 2; i <= m_max_val; i++) {
            if (!sieve.test(i)) continue;

            m_prime_numbers.emplace_back(i);

            if (i > 0 && i > UINT64_MAX / i) continue;

            uint64_t j = i * i;
            while (j <= m_max_val) {
                sieve.reset(j);
                j+=i;
            }
        }

        {
            std::lock_guard lock(m_factorizer_notifier.mx);
            m_factorizer_notifier.is_prime_numbers_prepared = true;
        }
        m_factorizer_notifier.cv.notify_all();
    }
} // workers