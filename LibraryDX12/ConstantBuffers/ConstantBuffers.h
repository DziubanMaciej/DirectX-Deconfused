#pragma once

#include <DXD/ExternalHeadersWrappers/DirectXMath.h>
#include <DXD/ExternalHeadersWrappers/windows.h>

// clang-format off
#include "ConstantBuffers/ConstantBuffersDefs.inl"
// clang-format on

// ---------------------------------------------------- Buffers for 3D shaders

struct ModelMvp {
    matrix modelMatrix;
    matrix modelViewProjectionMatrix;
};

struct NormalTextureCB {
    matrix modelMatrix;
    matrix modelViewProjectionMatrix;
    float2 textureScale;
};

struct TextureNormalMapCB {
    matrix modelMatrix;
    matrix modelViewProjectionMatrix;
    float2 textureScale;
    float _padding;
    uint normalMapAvailable;
};

struct ObjectPropertiesCB {
    float3 albedoColor;
    float specularity;
    float bloomFactor;
};

// ---------------------------------------------------- Buffers for post processes

struct PostProcessApplyBloomCB {
    float screenWidth;
    float screenHeight;
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
    float4x3 kernel; // We actually use 3x3, but 4x3 is required due to HLSL packing
    float screenWidth;
    float screenHeight;
    float divider;
};

struct PostProcessGaussianBlurComputeCB {
    float screenWidth;
    float screenHeight;
};

struct PostProcessGaussianBlurCB {
    float screenWidth;
    float screenHeight;
    uint samplingRange;
    bool horizontal;
};

struct PostProcessLinearColorCorrectionCB {
    float4x3 colorMatrix; // We actually use 3x3, but 4x3 is required due to HLSL packing
    float screenWidth;
    float screenHeight;
};

struct PostProcessFxaaCB {
    float screenWidth;
    float screenHeight;
};

// ---------------------------------------------------- Buffers for shadow maps

struct ShadowMapCB {
    matrix mvp;
};

// ---------------------------------------------------- Buffers for shadow maps

struct GenerateMipsCB {
    float2 texelSize;
    uint sourceMipLevel;
    bool isSrgb;
};

struct LightingHeapCB {
    float4 cameraPosition;
    float3 ambientLight;
    float padding[1];
    float shadowMapSize;
    float screenWidth;
    float screenHeight;
    int lightsSize;
    float4 lightPosition[8];
    float4 lightColor[8];
    float4 lightDirection[8];
    matrix smViewProjectionMatrix[8];
};

struct LightingCB {
    matrix viewMatrixInverse;
    matrix projMatrixInverse;
    uint enableSSAO;
};

struct FogCB {
    float3 fogColor;
    float screenWidth;
    float screenHeight;
    float fogPower;
    matrix viewMatrixInverse;
    matrix projMatrixInverse;
    float4 cameraPosition;
};

struct DofCB {
    float screenWidth;
    float screenHeight;
};

struct SpriteCB {
    float screenWidth;
    float screenHeight;
    float textureSizeX;
    float textureOffsetX;
    float textureSizeY;
    float textureOffsetY;
    uint horizontalAlignment;
    uint verticalAlignment;
};

struct SsaoCB {
    float screenWidth;
    float screenHeight;
    matrix viewMatrixInverse;
    matrix projMatrixInverse;
};

struct SsrCB {
    float4 cameraPosition;
    float screenWidth;
    float screenHeight;
    matrix viewMatrixInverse;
    matrix projMatrixInverse;
    matrix viewProjectionMatrix;
    float4 clearColor;
};

struct SsrMergeCB {
    float screenWidth;
    float screenHeight;
};

#include "ConstantBuffers/ConstantBuffersUndefs.inl"
