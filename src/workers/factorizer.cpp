//
// Created by Roman Yakimkin on 26.06.2026.
//

#include <iostream>
#include <thread>

#include "factorizer.h"

namespace workers {
    Factorizer::Factorizer(uint16_t thread_numb, NumberQueue *input_queue,
                           ResultQueue *output_queue, core::IPrimeNumbersProvider* prime_numbers,
                           PrimeNumbersGeneratedNotifier& notifier)
        : m_thread_numb(thread_numb), m_input_queue(input_queue),
          m_output_queue(output_queue), m_prime_numbers(prime_numbers),
          m_prime_numbers_generated_notifier(notifier)  {
    }

    std::vector<uint64_t> Factorizer::getPrimeFactors(uint64_t numb) const {
        std::vector<uint64_t> result;

        if (numb <= 1) return result;

        auto prime_numbers = m_prime_numbers->getPrimeNumbers();

        uint64_t pm_index = 0;

        for (uint64_t pm_current : prime_numbers) {
            if (pm_current * pm_current > numb) break;
            while (numb % pm_current == 0) {
                result.emplace_back(pm_current);
                numb /= pm_current;
            }

            if (numb == 1) break;
        }

        if (numb > 1) result.emplace_back(numb);

        return result;
    }

    void Factorizer::operator()() {
        std::cout << "Factorizer thread #" << m_thread_numb << ", native id: " << std::this_thread::get_id() << " started.\n";

        // Ждать, пока не закончит работу поток, генерирующий массив простых чисел
        {
            std::unique_lock lock(m_prime_numbers_generated_notifier.mx);
            m_prime_numbers_generated_notifier.cv.wait(lock, [this] {
                return m_prime_numbers_generated_notifier.is_prime_numbers_prepared;
            });
        }

        std::cout << "Factorized thread #" << m_thread_numb << " woke up\n";

        // Основной цикл
        try {
            while (auto number = m_input_queue->pop()) {

                auto start = std::chrono::steady_clock::now();
                auto factors = getPrimeFactors(number.value());
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

                m_output_queue->push({number.value(), std::move(factors), m_thread_numb, duration});
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Result performer Fatal error " << e.what() << "\n";
        }
        catch (...) {
            std::cerr << "[Thread] Unknown fatal error.\n";
        }

        std::cout << "Result performer thread stopped gracefully.\n";
    }
} // workers