//
// Created by Roman Yakimkin on 30.06.2026.
//

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "workers/prime_numbers_provider.h"

TEST(PrimeNumbersProviderTest, GeneratePrimesUpTo30) {
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider provider(30, notifier);

    provider();

    auto primes = provider.getPrimeNumbers();
    std::vector<uint64_t> expected = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};

    ASSERT_EQ(primes.size(), expected.size());
    for (size_t i = 0; i < expected.size(); i++) EXPECT_EQ(primes[i], expected[i]);
}

TEST(PrimeNumbersProviderTest, GeneratePrimesUpTo10) {
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider provider(10, notifier);

    provider();

    auto primes = provider.getPrimeNumbers();

    std::vector<uint64_t> expected = {2, 3, 5, 7};

    ASSERT_EQ(primes.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) EXPECT_EQ(primes[i], expected[i]);
}

TEST(PrimeNumbersProviderTest, GeneratePrimesUpTo2) {
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider provider(2, notifier);

    provider();
    auto primes = provider.getPrimeNumbers();

    ASSERT_EQ(primes.size(), 1);
    EXPECT_EQ(primes[0], 2);
}

TEST(PrimeNumbersProviderTest, GeneratePrimesUpTo1) {
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider provider(1, notifier);

    provider();
    auto primes = provider.getPrimeNumbers();

    EXPECT_EQ(primes.size(), 0);
}

TEST(PrimeNumbersProviderTest, NotifierIsSetAfterGeneration) {
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider provider(20, notifier);

    EXPECT_FALSE(notifier.is_prime_numbers_prepared);

    provider();

    EXPECT_TRUE(notifier.is_prime_numbers_prepared);
}

TEST(PrimeNumbersProviderTest, MultiThreadGeneration) {
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider provider(100, notifier);

    std::atomic generation_completed{false};

    std::thread generator([&provider, &generation_completed]() {
        provider();
        generation_completed = true;
    });

    generator.join();

    EXPECT_TRUE(generation_completed);
    EXPECT_TRUE(notifier.is_prime_numbers_prepared);

    auto primes = provider.getPrimeNumbers();

    EXPECT_EQ(primes.size(), 25);
    EXPECT_EQ(primes[0], 2);
    EXPECT_EQ(primes[24], 97);
}

TEST(PrimeNumbersProviderTest, WaitingForNotification) {
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider provider(50, notifier);

    std::atomic notification_received{false};

    std::thread generator([&provider]() {
        provider();
    });

    std::thread waiter([&notifier, &notification_received]() {
        std::unique_lock lock(notifier.mx);
        notifier.cv.wait(lock, [&notifier]() {
            return notifier.is_prime_numbers_prepared;
        });

        notification_received = true;
    });

    generator.join();
    waiter.join();

    EXPECT_TRUE(notification_received);
    EXPECT_TRUE(notifier.is_prime_numbers_prepared);
}

TEST(PrimeNumbersProviderTest, GeneratePrimesUpTo1000) {
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider provider(1000, notifier);

    provider();
    auto primes = provider.getPrimeNumbers();

    // До 1000 должно быть 168 простых чисел
    EXPECT_EQ(primes.size(), 168);

    // Проверяем первое и последнее
    EXPECT_EQ(primes[0], 2);
    EXPECT_EQ(primes[167], 997); // 997 - последнее простое до 1000
}

TEST(PrimeNumbersProviderTest, AllGeneratedNumbersArePrime) {
    workers::PrimeNumbersGeneratedNotifier notifier;
    workers::PrimeNumbersProvider provider(200, notifier);

    provider();
    auto primes = provider.getPrimeNumbers();

    // Проверяем каждое число
    for (uint64_t prime : primes) {
        // Простое число должно делиться только на 1 и на себя
        int divisors = 0;
        for (uint64_t i = 1; i <= prime; ++i) {
            if (prime % i == 0) divisors++;
        }
        EXPECT_EQ(divisors, 2) << "Number " << prime << " is not prime";
    }
}

