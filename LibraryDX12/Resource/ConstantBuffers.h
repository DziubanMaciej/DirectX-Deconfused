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

struct PostProcessGaussianBlurData {
    PostProcessGaussianBlurCB cb;
    UINT passCount;
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
