#include "DXD/ExternalHeadersWrappers/DirectXMath.h"

struct SimpleConstantBuffer {
    XMMATRIX mvpMatrix;
    int lightsSize;
    XMFLOAT3 lightPosition;
    XMFLOAT3 lightColor;
};