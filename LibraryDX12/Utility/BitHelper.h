#pragma once

#include "DXD/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/windows.h>

struct BitHelper : DXD::NonCopyableAndMovable {
    BitHelper() = delete;

    constexpr static bool isPowerOfTwo(UINT arg) {
        int bitsSet = 0;
        for (int i = 0; i < sizeof(arg) * 8; i++) {
            bitsSet += (arg & 0x1);
            arg >>= 1;
        }
        return bitsSet == 1;
    }

    template <UINT alignment>
    constexpr static UINT alignUp(UINT arg) {
        static_assert(isPowerOfTwo(alignment), "Can only align to power of two");
        return (arg + alignment - 1) & ~(alignment - 1);
    }
};
