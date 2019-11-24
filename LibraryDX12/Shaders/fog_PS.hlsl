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

float4 getPositionFromDepth(float u, float v, float depth) {
    float4 H = float4((u)*2 - 1, (1 - v) * 2 - 1, depth, 1.0);
    float4 D = mul(scb.projMatrixInverse, H);
    float4 INworldPosition = mul(scb.viewMatrixInverse, (D / D.w));
    return INworldPosition;
}

float4 getNonLinearDepth(float u, float v, float depth) {
    float4 H = float4(0, 0, depth, 1.0);
    float4 D = mul(scb.projMatrixInverse, H);
    return D;
}

float4 main(PixelShaderInput IN) : SV_Target {

    const float uBase = IN.Position.x / scb.screenWidth;
    const float vBase = IN.Position.y / scb.screenHeight;

    float INdepth = gBufferDepth.Sample(g_sampler, float2(uBase, vBase)).r;

    float3 INlightingOutput = lightingOutput.Sample(g_sampler, float2(uBase, vBase)).rgb;

    float4 position = getPositionFromDepth(uBase, vBase, INdepth);
    
    float fogPower = smoothstep(1.0f - scb.fogPower, 1.0f, INdepth);

    float3 fogColor = scb.fogColor.xyz;
    

    float3 output = INlightingOutput * (1.0f - fogPower) + fogColor * fogPower;

    return float4(output,1);
}
