#pragma once

#include "DXD/Export.h"

struct EXPORT NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
};

struct EXPORT NonMovable {
    NonMovable() = default;
    NonMovable(NonMovable &&) = delete;
    NonMovable &operator=(NonMovable &&) = delete;
};

struct EXPORT NonCopyableAndMovable : NonCopyable, NonMovable {
    NonCopyableAndMovable() = default;
};
