//
// Created by Roman Yakimkin on 30.06.2026.
//

#include <gtest/gtest.h>
#include "libs/bit_array.h"

TEST(BitArrayTest, InitialState) {
    BitArray ba(100);
    EXPECT_EQ(ba.size(), 100);
}

TEST(BitArrayTest, SetAndReset) {
    BitArray ba(100);

    ba.reset(2);
    EXPECT_FALSE(ba.test(2));

    ba.set(2);
    EXPECT_TRUE(ba.test(2));

    ba.set(63);
    EXPECT_TRUE(ba.test(63));

    ba.set(64);
    EXPECT_TRUE(ba.test(64));
}