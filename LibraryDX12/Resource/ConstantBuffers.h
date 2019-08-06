#include "DXD/ExternalHeadersWrappers/DirectXMath.h"

struct ModelMvp {
    XMMATRIX modelMatrix;
    XMMATRIX modelViewProjectionMatrix;
};

struct PostProcessCB {
    float screenWidth;
    float screenHeight;
};

struct ObjectProperties {
    XMFLOAT3 objectColor;
    float objectSpecularity;
};

struct SimpleConstantBuffer { //must be 128bit padded
    XMFLOAT4 cameraPosition;
    int lightsSize;
    XMFLOAT3 ambientLight;
    XMFLOAT4 lightPosition[8];
    XMFLOAT4 lightColor[8];
    XMFLOAT4 lightDirection[8];
};