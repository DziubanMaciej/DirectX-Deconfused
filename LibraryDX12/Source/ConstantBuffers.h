#include "DXD/ExternalHeadersWrappers/DirectXMath.h"

struct ModelMvp {
    XMMATRIX modelMatrix;
    XMMATRIX modelViewProjectionMatrix;
};

struct SimpleConstantBuffer { //must be 128bit padded
    int lightsSize;
    int pad;
    int padd;
    int paddd;
    XMFLOAT4 lightPosition[8];
    XMFLOAT4 lightColor[8];
};