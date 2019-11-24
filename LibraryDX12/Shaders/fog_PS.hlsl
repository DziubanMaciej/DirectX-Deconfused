Texture2D gBufferDepth : register(t0);
Texture2D lightingOutput : register(t1);

SamplerState g_sampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
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

ConstantBuffer<FogCB> scb : register(b1);


float4 main(PixelShaderInput IN) : SV_Target {

    const float uBase = IN.Position.x / scb.screenWidth;
    const float vBase = IN.Position.y / scb.screenHeight;

    float INdepth = gBufferDepth.Sample(g_sampler, float2(uBase, vBase)).r;

    float3 fogColor = scb.fogColor.xyz;

    if (INdepth >= 1.0f) {
        return float4(fogColor, 1.0f);
    }

    float3 INlightingOutput = lightingOutput.Sample(g_sampler, float2(uBase, vBase)).rgb;
    
    float fogPower = smoothstep(1.0f - clamp(scb.fogPower, 0.0f, 1.0f), 1.0f, INdepth);

    float3 output = INlightingOutput * (1.0f - fogPower) + fogColor * fogPower;

    return float4(output,1);
}
