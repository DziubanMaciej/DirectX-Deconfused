#pragma once

#include "DXD/ExternalHeadersWrappers/DirectXMath.h"

struct ModelMvp {
    XMMATRIX modelMatrix;
    XMMATRIX modelViewProjectionMatrix;
};

struct PostProcessBlackBarsCB {
    float screenWidth;
    float screenHeight;
    float leftMarginPercent;
    float rightMarginPercent;
    float topMarginPercent;
    float bottomMarginPercent;
};

struct PostProcessConvolutionCB {
    XMFLOAT4X3 kernel; // We actually use 3x3, but 4x3 is required due to HLSL packing
    float screenWidth;
    float screenHeight;
    float divider;
};

struct PostProcessLinearColorCorrectionCB {
    XMFLOAT4X3 colorMatrix; // We actually use 3x3, but 4x3 is required due to HLSL packing
    float screenWidth;
    float screenHeight;
};

struct PostProcessGaussianBlurCB {
    float screenWidth;
    float screenHeight;
    UINT samplingRange;
    bool horizontal;
    bool padding[3];
};

struct PostProcessApplyBloomCB {
    float screenWidth;
    float screenHeight;
};

struct SsaoCB {
    float screenWidth;
    float screenHeight;
    XMMATRIX viewMatrixInverse;
    XMMATRIX projMatrixInverse;
};

struct PostProcessGaussianBlurData {
    PostProcessGaussianBlurCB cb;
    UINT passCount;
};

struct ObjectProperties {
    XMFLOAT3 objectColor;
    float objectSpecularity;
};

struct LightingConstantBuffer {
    XMFLOAT4 cameraPosition;
    int lightsSize;
    XMFLOAT3 ambientLight;
    XMFLOAT4 lightPosition[8];
    XMFLOAT4 lightColor[8];
    XMFLOAT4 lightDirection[8];
    XMMATRIX smViewProjectionMatrix[8];
    float screenWidth;
    float screenHeight;
};

struct SMmvp {
    XMMATRIX modelViewProjectionMatrix;
};

struct InverseViewProj {
    XMMATRIX viewMatrixInverse;
    XMMATRIX projMatrixInverse;
};
