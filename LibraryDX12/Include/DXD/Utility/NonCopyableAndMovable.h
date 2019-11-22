#pragma once

#include "DXD/Utility/Export.h"

namespace DXD {
/// @cond
struct EXPORT NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
    NonCopyable(NonCopyable &&) = default;
    NonCopyable &operator=(NonCopyable &&) = default;
};

struct EXPORT NonMovable {
    NonMovable() = default;
    NonMovable(const NonMovable &) = default;
    NonMovable &operator=(const NonMovable &) = default;
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

struct EXPORT NonInstantiatable {
    NonInstantiatable() = delete;
    NonInstantiatable(const NonInstantiatable &) = delete;
    NonInstantiatable &operator=(const NonInstantiatable &) = delete;
    NonInstantiatable(NonInstantiatable &&) = delete;
    NonInstantiatable &operator=(NonInstantiatable &&) = delete;
};

/// @endcond
} // namespace DXD
