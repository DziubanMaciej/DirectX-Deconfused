#pragma once

#include "DXD/Utility/Export.h"

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
    NonCopyableAndMovable(const NonCopyableAndMovable &) = delete;
    NonCopyableAndMovable &operator=(const NonCopyableAndMovable &) = delete;
    NonCopyableAndMovable(NonCopyableAndMovable &&) = delete;
    NonCopyableAndMovable &operator=(NonCopyableAndMovable &&) = delete;
};
} // namespace DXD
