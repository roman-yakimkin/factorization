//
// Created by Roman Yakimkin on 30.06.2026.
//

#include <gtest/gtest.h>
#include "workers/bit_sieve.h"

TEST(BitSieveTest, InitialState) {
    BitSieve sieve(100);

    // По логике конструктора, биты 0 и 1 должны быть сброшены (false)
    EXPECT_FALSE(sieve.test(0));
    EXPECT_FALSE(sieve.test(1));

    // Остальные биты изначально установлены (true), так как m_data заполняется 0xFF...FF
    EXPECT_TRUE(sieve.test(2));
    EXPECT_TRUE(sieve.test(63)); // Граница первого 64-битного слова
    EXPECT_TRUE(sieve.test(64)); // Начало второго 64-битного слова
}

TEST(BitSieveTest, BoundaryConditions) {
    BitSieve sieve(64);

    EXPECT_EQ(sieve.size(), 64);
    EXPECT_FALSE(sieve.test(0));
    EXPECT_FALSE(sieve.test(1));
    EXPECT_TRUE(sieve.test(63));
}