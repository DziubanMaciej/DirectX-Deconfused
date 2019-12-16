#pragma once

#include <DirectXMath.h>
using namespace DirectX;

namespace DirectX {
inline XMFLOAT3 XM_CALLCONV XMStoreFloat3(FXMVECTOR v) {
    XMFLOAT3 result;
    XMStoreFloat3(&result, v);
    return result;
}

inline XMFLOAT4 toXmFloat4(XMFLOAT3 v, float w) {
    return XMFLOAT4(v.x, v.y, v.z, w);
}

} // namespace DirectX
