struct GammaCorrectionCB {
    float screenWidth;
    float screenHeight;
    float gammaValue;
};
ConstantBuffer<GammaCorrectionCB> cb : register(b0);
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

    float4 color = scene.Sample(sceneSampler, float2(uBase, vBase));
    if (cb.gammaValue > 1) {
        color.rgb = pow(color.rgb, cb.gammaValue);
    }
    return float4(color);
}
