#include "Utility/MathHelper.h"

#include <gtest/gtest.h>

TEST(MathHelperTests, givenPowersOfTwoWhenCallingIsPowerOfTwoTheReturnTrue) {
    EXPECT_TRUE(MathHelper::isPowerOfTwo(1));
    EXPECT_TRUE(MathHelper::isPowerOfTwo(2));
    EXPECT_TRUE(MathHelper::isPowerOfTwo(4));
    EXPECT_TRUE(MathHelper::isPowerOfTwo(8));
    EXPECT_TRUE(MathHelper::isPowerOfTwo(16));
    EXPECT_TRUE(MathHelper::isPowerOfTwo(256));
}

TEST(MathHelperTests, givenZeroWhenCallingIsPowerOfTwoThenReturnFalse) {
    EXPECT_FALSE(MathHelper::isPowerOfTwo(0));
}

TEST(MathHelperTests, givenNonPowersOfTwoWhenCallingIsPowerOfTwoThenReturnTrue) {
    EXPECT_FALSE(MathHelper::isPowerOfTwo(3));
    EXPECT_FALSE(MathHelper::isPowerOfTwo(5));
    EXPECT_FALSE(MathHelper::isPowerOfTwo(6));
    EXPECT_FALSE(MathHelper::isPowerOfTwo(7));
    EXPECT_FALSE(MathHelper::isPowerOfTwo(11));
    EXPECT_FALSE(MathHelper::isPowerOfTwo(12));
    EXPECT_FALSE(MathHelper::isPowerOfTwo(15));
}

TEST(MathHelperTests, givenAlreadyAlignedNumberWhenCallingAlignUpThenReturnIt) {
    EXPECT_EQ(1, MathHelper::alignUp<1>(1));
    EXPECT_EQ(2, MathHelper::alignUp<2>(2));
    EXPECT_EQ(4, MathHelper::alignUp<4>(4));
    EXPECT_EQ(8, MathHelper::alignUp<8>(8));
    EXPECT_EQ(16, MathHelper::alignUp<16>(16));
}

TEST(MathHelperTests, givenNotAlignedNumberWheneCallingAlignUpThenReturnCorrectlyAlignedValue) {
    EXPECT_EQ(36, MathHelper::alignUp<4>(33));
    EXPECT_EQ(36, MathHelper::alignUp<4>(34));
    EXPECT_EQ(36, MathHelper::alignUp<4>(35));
}


TEST(MathHelperTests, givenVariousInputsWhenCallingDivideByMultipleThenReturnCorrectResults) {
    EXPECT_EQ(1, MathHelper::divideByMultiple(7, 8));
    EXPECT_EQ(1, MathHelper::divideByMultiple(8, 8));

    EXPECT_EQ(2, MathHelper::divideByMultiple(9, 8));
    EXPECT_EQ(2, MathHelper::divideByMultiple(10, 8));
    EXPECT_EQ(2, MathHelper::divideByMultiple(16, 8));

    EXPECT_EQ(3, MathHelper::divideByMultiple(17, 8));
    EXPECT_EQ(3, MathHelper::divideByMultiple(18, 8));
    EXPECT_EQ(3, MathHelper::divideByMultiple(24, 8));
}

TEST(MathHelperTests, givenInputsToClampThenItReturnsCorrectResults) {
    const auto min = 10;
    const auto max = 20;

    EXPECT_EQ(min, MathHelper::clamp(0, min, max));
    EXPECT_EQ(min, MathHelper::clamp(9, min, max));
    EXPECT_EQ(min, MathHelper::clamp(10, min, max));

    EXPECT_EQ(11, MathHelper::clamp(11, min, max));
    EXPECT_EQ(13, MathHelper::clamp(13, min, max));
    EXPECT_EQ(19, MathHelper::clamp(19, min, max));

    EXPECT_EQ(max, MathHelper::clamp(max, min, max));
    EXPECT_EQ(max, MathHelper::clamp(21, min, max));
    EXPECT_EQ(max, MathHelper::clamp(26, min, max));
}