#include "Resource/Resource.h"
#include "DXD/Application.h"

#include <gtest/gtest.h>

TEST(a, a) {
    EXPECT_EQ(1, 2 - 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
