struct PostProcessGaussianBlurCB {
    float screenWidth;
    float screenHeight;
    uint samplingRange;
    bool horizontal;
};
ConstantBuffer<PostProcessGaussianBlurCB> cb : register(b0);
Texture2D scene : register(t0);
SamplerState sceneSampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    const float weights[] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};

    const float uBase = IN.Position.x / cb.screenWidth;
    const float vBase = IN.Position.y / cb.screenHeight;
    const float uOffset = 1 / cb.screenWidth;
    const float vOffset = 1 / cb.screenHeight;

    float4 OUT_Color = scene.Sample(sceneSampler, float2(uBase, vBase)) * weights[0]; // current pixel's contribution
    if (cb.horizontal) {
        for (int i = 1; i < cb.samplingRange; i++) {
            OUT_Color.rgb += weights[i] * scene.Sample(sceneSampler, float2(uBase - i * uOffset, vBase)).rgb;
            OUT_Color.rgb += weights[i] * scene.Sample(sceneSampler, float2(uBase + i * uOffset, vBase)).rgb;
        }
    } else {
        for (int i = 1; i < cb.samplingRange; i++) {
            OUT_Color.rgb += weights[i] * scene.Sample(sceneSampler, float2(uBase, vBase + i * vOffset)).rgb;
            OUT_Color.rgb += weights[i] * scene.Sample(sceneSampler, float2(uBase, vBase - i * vOffset)).rgb;
        }
    }
    return OUT_Color;
}
