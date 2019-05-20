#pragma once

#include "DXD/Export.h"

namespace DXD {
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

struct EXPORT NonCopyableAndMovable {
    NonCopyableAndMovable() = default;
    NonCopyableAndMovable(const NonCopyable &) = delete;
    NonCopyableAndMovable &operator=(const NonCopyable &) = delete;
    NonCopyableAndMovable(NonMovable &&) = delete;
    NonCopyableAndMovable &operator=(NonMovable &&) = delete;
};
} // namespace DXD
