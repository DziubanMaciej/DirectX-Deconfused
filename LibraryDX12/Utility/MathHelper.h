#pragma once

#include "DXD/ExternalHeadersWrappers/windows.h"

#include <algorithm>

namespace MathHelper {

template <typename T>
constexpr inline bool isPowerOfTwo(T arg) {
    int bitsSet = 0;
    for (int i = 0; i < sizeof(arg) * 8; i++) {
        bitsSet += (arg & 0x1);
        arg >>= 1;
    }
    return bitsSet == 1;
}

template <typename T, T alignment>
constexpr inline static T alignUp(T arg) {
    static_assert(isPowerOfTwo(alignment), "Can only align to power of two");
    return (arg + alignment - 1) & ~(alignment - 1);
}

template <UINT alignment>
constexpr inline static UINT alignUp(UINT arg) {
    return alignUp<UINT, alignment>(arg);
}

template <UINT64 alignment>
constexpr inline static UINT64 alignUp(UINT64 arg) {
    return alignUp<UINT64, alignment>(arg);
}

template <typename T>
constexpr inline T divideByMultiple(T value, size_t alignment) {
    return (T)((value + alignment - 1) / alignment);
}

template <typename T>
constexpr inline T clamp(const T &n, const T &lower, const T &upper) {
    return std::max(lower, std::min(n, upper));
}

} // namespace MathHelper
