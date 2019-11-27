#define FXAA_PC 1
#define FXAA_QUALITY__PRESET 23
//#define FXAA_GREEN_AS_LUMA 1
#define FXAA_HLSL_5 1
#include "Fxaa3_11.h"
struct PostProcessFxaaCB {
    float screenWidth;
    float screenHeight;
};

ConstantBuffer<PostProcessFxaaCB> cb : register(b0);
Texture2D scene : register(t0);
SamplerState g_sampler : register(s0);
struct PixelShaderInput {
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    const float uBase = IN.Position.x / cb.screenWidth;
    const float vBase = IN.Position.y / cb.screenHeight;

    FxaaTex tex;
    tex.smpl = g_sampler;
    tex.tex = scene;

    return FxaaPixelShader(
        float2(uBase, vBase),                                // FxaaFloat2 pos,
        float4(0.0f, 0.0f, 0.0f, 0.0f),                      // FxaaFloat4 fxaaConsolePosPos,
        tex,                                                 // FxaaTex tex,
        tex,                                                 // FxaaTex fxaaConsole360TexExpBiasNegOne,
        tex,                                                 // FxaaTex fxaaConsole360TexExpBiasNegTwo,
        float2(1.f / cb.screenWidth, 1.f / cb.screenHeight), // FxaaFloat2 fxaaQualityRcpFrame,
        float4(0.0f, 0.0f, 0.0f, 0.0f),                      // FxaaFloat4 fxaaConsoleRcpFrameOpt,
        float4(0.0f, 0.0f, 0.0f, 0.0f),                      // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
        float4(0.0f, 0.0f, 0.0f, 0.0f),                      // FxaaFloat4 fxaaConsole360RcpFrameOpt2,
        0.75f,                                               // FxaaFloat fxaaQualitySubpix,
        0.166f,                                              // FxaaFloat fxaaQualityEdgeThreshold,
        0.0833f,                                             // FxaaFloat fxaaQualityEdgeThresholdMin,
        0.0f,                                                // FxaaFloat fxaaConsoleEdgeSharpness,
        0.0f,                                                // FxaaFloat fxaaConsoleEdgeThreshold,
        0.0f,                                                // FxaaFloat fxaaConsoleEdgeThresholdMin,
        float4(0.0f, 0.0f, 0.0f, 0.0f)                       // FxaaFloat fxaaConsole360ConstDir,
    );
}
