struct PostProcessConvolutionCB {
    float4x3 kernel;
    float screenWidth;
    float screenHeight;
    float sum;
};
ConstantBuffer<PostProcessConvolutionCB> cb : register(b0);
Texture2D scene : register(t0);
SamplerState sceneSampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    const float uBase = IN.Position.x / cb.screenWidth;
    const float vBase = IN.Position.y / cb.screenHeight;
    const float uOffset = 1 / cb.screenWidth;
    const float vOffset = 1 / cb.screenHeight;

    float4 OUT_Color = float4(0, 0, 0, 1.f);
    OUT_Color.rgb += cb.kernel[0][0] * scene.Sample(sceneSampler, float2(uBase - uOffset, vBase - vOffset));
    OUT_Color.rgb += cb.kernel[0][1] * scene.Sample(sceneSampler, float2(uBase - uOffset, vBase));
    OUT_Color.rgb += cb.kernel[0][2] * scene.Sample(sceneSampler, float2(uBase - uOffset, vBase + vOffset));
    OUT_Color.rgb += cb.kernel[1][0] * scene.Sample(sceneSampler, float2(uBase, vBase - vOffset));
    OUT_Color.rgb += cb.kernel[1][1] * scene.Sample(sceneSampler, float2(uBase, vBase));
    OUT_Color.rgb += cb.kernel[1][2] * scene.Sample(sceneSampler, float2(uBase, vBase + vOffset));
    OUT_Color.rgb += cb.kernel[2][0] * scene.Sample(sceneSampler, float2(uBase + uOffset, vBase - vOffset));
    OUT_Color.rgb += cb.kernel[2][1] * scene.Sample(sceneSampler, float2(uBase + uOffset, vBase));
    OUT_Color.rgb += cb.kernel[2][2] * scene.Sample(sceneSampler, float2(uBase + uOffset, vBase + vOffset));
    OUT_Color.rgb /= cb.sum;
    return float4(OUT_Color);
}
