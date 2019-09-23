struct PostProcessCB {
    float screenWidth;
    float screenHeight;
};

ConstantBuffer<PostProcessCB> cb : register(b0);
Texture2D bloomMap : register(t0);
SamplerState g_sampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    const float uBase = IN.Position.x / cb.screenWidth;
    const float vBase = IN.Position.y / cb.screenHeight;
    return bloomMap.Sample(g_sampler, float2(uBase, vBase));
}
