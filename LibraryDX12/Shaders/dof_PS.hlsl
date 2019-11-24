Texture2D gBufferDepth : register(t0);
Texture2D lightingOutput : register(t1);

SamplerState g_sampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

struct DofCB {
    float screenWidth;
    float screenHeight;
};

ConstantBuffer<DofCB> scb : register(b1);


float4 main(PixelShaderInput IN) : SV_Target {

    const float uBase = IN.Position.x / scb.screenWidth;
    const float vBase = IN.Position.y / scb.screenHeight;

    float INdepth = gBufferDepth.Sample(g_sampler, float2(uBase, vBase)).r;

    float3 INlightingOutput = lightingOutput.Sample(g_sampler, float2(uBase, vBase)).rgb;

    return float4(INdepth, 0, 0, 1);
}
