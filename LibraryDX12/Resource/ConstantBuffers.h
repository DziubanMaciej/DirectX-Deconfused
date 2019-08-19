#include "DXD/ExternalHeadersWrappers/DirectXMath.h"

struct ModelMvp {
    XMMATRIX modelMatrix;
    XMMATRIX modelViewProjectionMatrix;
};

struct PostProcessCB {
    float screenWidth;
    float screenHeight;
};

struct PostProcessConvolutionCB {
    XMFLOAT4X3 kernel; // We actually use 3x3, but 4x3 is required due to HLSL packing
    float screenWidth;
    float screenHeight;
    float sum;
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
    XMMATRIX smViewProjectionMatrix[8];
};

struct SMmvp {
    XMMATRIX modelViewProjectionMatrix;
};