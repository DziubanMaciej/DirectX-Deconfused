﻿#include "Application/ApplicationImpl.h"

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    auto application = DXD::Application::create(false, false, DXD::Application::MinimizeBehavior::Keep); // TODO this is sacrilege
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
