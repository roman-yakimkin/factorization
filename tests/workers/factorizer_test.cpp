//
// Created by Roman Yakimkin on 01.07.2026.
//
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <vector>

#include "workers/factorizer.h"
#include "workers/prime_numbers_provider.h"
#include "core/models/factorized_number.h"

TEST(FactorizerTest, FactorizeSingleNumber) {
    ConcurrentQueue<uint64_t> input_queue;
    ConcurrentQueue<model::FactorizedNumber> output_queue;
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider pn_provider(1000, notifier);
    pn_provider();

    workers::Factorizer factorizer(1, &input_queue, &output_queue, &pn_provider, notifier);

    input_queue.push(100);
    input_queue.close();

    std::thread factorizer_thread(std::ref(factorizer));
    factorizer_thread.join();

    output_queue.close();

    auto result = output_queue.pop();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().m_numb, 100);
    EXPECT_EQ(result.value().m_thread_numb, 1);

    std::vector<uint64_t> expected_factors = {2, 2, 5, 5};
    ASSERT_EQ(result.value().m_factors.size(), expected_factors.size());
    for (size_t i = 0; i < expected_factors.size(); ++i) {
        EXPECT_EQ(result.value().m_factors[i], expected_factors[i]);
    }
}

TEST(FactorizerTest, FactorizePrimeNumber) {
    ConcurrentQueue<uint64_t> input_queue;
    ConcurrentQueue<model::FactorizedNumber> output_queue;

    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider prime_provider(1000, notifier);
    prime_provider();

    workers::Factorizer factorizer(1, &input_queue, &output_queue, &prime_provider, notifier);

    input_queue.push(17); // Простое число
    input_queue.close();

    std::thread factorizer_thread(std::ref(factorizer));

    factorizer_thread.join();
    output_queue.close();

    auto result = output_queue.pop();
    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result.value().m_numb, 17);
    ASSERT_EQ(result.value().m_factors.size(), 1);
    EXPECT_EQ(result.value().m_factors[0], 17);
}

TEST(FactorizerTest, FactorizeMultipleNumbers) {
    ConcurrentQueue<uint64_t> input_queue;
    ConcurrentQueue<model::FactorizedNumber> output_queue;
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider prime_provider(1000, notifier);
    prime_provider();

    workers::Factorizer factorizer(1, &input_queue, &output_queue, &prime_provider, notifier);

    std::vector<uint64_t> numbers = {12, 30, 100, 255};
    for (uint64_t num : numbers) {
        input_queue.push(num);
    }
    input_queue.close();

    std::thread factorizer_thread(std::ref(factorizer));

    factorizer_thread.join();
    output_queue.close();

    // Проверяем все результаты
    std::vector<model::FactorizedNumber> results;
    while (true) {
        auto val = output_queue.pop();
        if (!val.has_value()) break;
        results.push_back(val.value());
    }

    ASSERT_EQ(results.size(), numbers.size());

    // Проверяем 12 = 2 * 2 * 3
    auto it12 = std::ranges::find_if(results,
                                     [](const auto& r) { return r.m_numb == 12; });
    ASSERT_NE(it12, results.end());
    EXPECT_EQ(it12->m_factors, (std::vector<uint64_t>{2, 2, 3}));

    // Проверяем 30 = 2 * 3 * 5
    auto it30 = std::ranges::find_if(results,
                                     [](const auto& r) { return r.m_numb == 30; });
    ASSERT_NE(it30, results.end());
    EXPECT_EQ(it30->m_factors, (std::vector<uint64_t>{2, 3, 5}));

    // Проверяем 100 = 2 * 2 * 5 * 5
    auto it100 = std::ranges::find_if(results,
                                      [](const auto& r) { return r.m_numb == 100; });
    ASSERT_NE(it100, results.end());
    EXPECT_EQ(it100->m_factors, (std::vector<uint64_t>{2, 2, 5, 5}));

    // Проверяем 255 = 3 * 5 * 17
    auto it255 = std::ranges::find_if(results,
                                      [](const auto& r) { return r.m_numb == 255; });
    ASSERT_NE(it255, results.end());
    EXPECT_EQ(it255->m_factors, (std::vector<uint64_t>{3, 5, 17}));
}

TEST(FactorizerTest, FactorizeLargeNumber) {
    ConcurrentQueue<uint64_t> input_queue;
    ConcurrentQueue<model::FactorizedNumber> output_queue;
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider prime_provider(100000, notifier);
    prime_provider();

    workers::Factorizer factorizer(1, &input_queue, &output_queue, &prime_provider, notifier);

    // 999999937 - простое число
    input_queue.push(999999937);
    input_queue.close();

    std::thread factorizer_thread(std::ref(factorizer));

    factorizer_thread.join();
    output_queue.close();

    auto result = output_queue.pop();
    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result.value().m_numb, 999999937);
    ASSERT_EQ(result.value().m_factors.size(), 1);
    EXPECT_EQ(result.value().m_factors[0], 999999937);
}

TEST(FactorizerTest, FactorizeWithLargeFactors) {
    ConcurrentQueue<uint64_t> input_queue;
    ConcurrentQueue<model::FactorizedNumber> output_queue;

    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider prime_provider(100000, notifier);
    prime_provider();

    workers::Factorizer factorizer(1, &input_queue, &output_queue, &prime_provider, notifier);

    // 99991 * 99991 = 9998200081
    input_queue.push(9998200081ULL);
    input_queue.close();

    std::thread factorizer_thread(std::ref(factorizer));

    factorizer_thread.join();
    output_queue.close();

    auto result = output_queue.pop();
    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result.value().m_numb, 9998200081ULL);
    ASSERT_EQ(result.value().m_factors.size(), 2);
    EXPECT_EQ(result.value().m_factors[0], 99991);
    EXPECT_EQ(result.value().m_factors[1], 99991);
}

TEST(FactorizerTest, StopWithStopToken) {
    ConcurrentQueue<uint64_t> input_queue;
    ConcurrentQueue<model::FactorizedNumber> output_queue;
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider prime_provider(1000, notifier);
    prime_provider();

    workers::Factorizer factorizer(1, &input_queue, &output_queue, &prime_provider, notifier);

    std::atomic factorizer_stopped{false};

    std::thread factorizer_thread([&factorizer, &factorizer_stopped]() {
        factorizer();
        factorizer_stopped = true;
    });

    // Даём время запуститься
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    input_queue.close();

    // Запрашиваем остановку
    factorizer_thread.join();
    output_queue.close();

    EXPECT_TRUE(factorizer_stopped);
}

TEST(FactorizerTest, MultiThreadedFactorization) {
    ConcurrentQueue<uint64_t> input_queue;
    ConcurrentQueue<model::FactorizedNumber> output_queue;
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider prime_provider(10000, notifier);
    prime_provider();

    workers::Factorizer factorizer(1, &input_queue, &output_queue, &prime_provider, notifier);

    const int CNT_ITEMS = 100;

    // Producer поток
    std::thread producer([&input_queue]() {
        for (int i = 2; i < CNT_ITEMS + 2; ++i) {
            input_queue.push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        input_queue.close();
    });

    // Factorizer поток
    std::thread factorizer_thread(std::ref(factorizer));

    producer.join();
    factorizer_thread.join();
    output_queue.close();

    // Считаем результаты
    std::vector<model::FactorizedNumber> results;
    while (true) {
        auto val = output_queue.pop();
        if (!val.has_value()) break;
        results.push_back(val.value());
    }

    // Проверяем, что все числа обработаны
    ASSERT_EQ(results.size(), CNT_ITEMS);

    // Проверяем корректность факторизации для нескольких чисел
    for (const auto& result : results) {
        // Проверяем, что произведение множителей равно исходному числу
        uint64_t product = 1;
        for (uint64_t factor : result.m_factors) {
            product *= factor;
        }
        EXPECT_EQ(product, result.m_numb)
            << "Factorization failed for " << result.m_numb;
    }
}

TEST(FactorizerTest, AllFactorsArePrime) {
    ConcurrentQueue<uint64_t> input_queue;
    ConcurrentQueue<model::FactorizedNumber> output_queue;

    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider prime_provider(10000, notifier);
    prime_provider();

    workers::Factorizer factorizer(1, &input_queue, &output_queue, &prime_provider, notifier);

    std::vector<uint64_t> numbers = {120, 252, 1024, 9973};
    for (uint64_t num : numbers) input_queue.push(num);
    input_queue.close();

    std::thread factorizer_thread(std::ref(factorizer));

    factorizer_thread.join();
    output_queue.close();

    // Проверяем все результаты
    while (true) {
        auto result = output_queue.pop();
        if (!result.has_value()) break;

        // Проверяем, что каждый множитель - простое число
        for (uint64_t factor : result.value().m_factors) {
            // Простое число должно делиться только на 1 и на себя
            int divisors = 0;
            for (uint64_t i = 1; i <= factor; ++i) {
                if (factor % i == 0) divisors++;
            }
            EXPECT_EQ(divisors, 2)
                << "Factor " << factor << " of number " << result.value().m_numb
                << " is not prime";
        }
    }
}

TEST(FactorizerTest, FactorizeNumberOne) {
    ConcurrentQueue<uint64_t> input_queue;
    ConcurrentQueue<model::FactorizedNumber> output_queue;
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider prime_provider(1000, notifier);
    prime_provider();

    workers::Factorizer factorizer(1, &input_queue, &output_queue, &prime_provider, notifier);

    input_queue.push(1);
    input_queue.close();

    std::thread factorizer_thread(std::ref(factorizer));

    factorizer_thread.join();
    output_queue.close();

    auto result = output_queue.pop();
    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result.value().m_numb, 1);
    EXPECT_TRUE(result.value().m_factors.empty()); // У 1 нет множителей
}