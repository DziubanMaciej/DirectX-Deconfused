#pragma once

#include <DirectXMath.h>
using namespace DirectX;

namespace DirectX {
inline XMFLOAT3 XM_CALLCONV XMStoreFloat3(FXMVECTOR v) {
    XMFLOAT3 result;
    XMStoreFloat3(&result, v);
    return result;
}
} // namespace DirectX
