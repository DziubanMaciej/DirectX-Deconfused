struct PostProcessBlackBarsCB {
    float screenWidth;
    float screenHeight;
    float leftMarginPercent;
    float rightMarginPercent;
    float topMarginPercent;
    float bottomMarginPercent;
};

ConstantBuffer<PostProcessBlackBarsCB> cb : register(b0);
Texture2D scene : register(t0);
SamplerState g_sampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    const float uBase = IN.Position.x / cb.screenWidth;
    const float vBase = IN.Position.y / cb.screenHeight;

    if (vBase < cb.topMarginPercent || vBase > 1 - cb.bottomMarginPercent || uBase < cb.leftMarginPercent || uBase > 1 - cb.rightMarginPercent) {
        return float4(0, 0, 0, 1);
    }

    return scene.Sample(g_sampler, float2(uBase, vBase));
}
