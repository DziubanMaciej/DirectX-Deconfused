
Texture2D gBufferNormal : register(t0);
Texture2D gBufferDepth : register(t1);
Texture2D gBufferSpecular : register(t2);
Texture2D prevFrame : register(t3);

SamplerState g_sampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

struct SsrCB {
    float screenWidth;
    float screenHeight;
    matrix viewMatrixInverse;
    matrix projMatrixInverse;
};

ConstantBuffer<SsrCB> scb : register(b1);

float4 getPositionFromDepth(float u, float v, float depth) {
    float4 H = float4((u)*2 - 1, (1 - v) * 2 - 1, depth, 1.0);
    float4 D = mul(scb.projMatrixInverse, H);
    float4 INworldPosition = mul(scb.viewMatrixInverse, (D / D.w));
    return INworldPosition;
}

float4 main(PixelShaderInput IN) : SV_Target {

    const float uBase = IN.Position.x / scb.screenWidth;
    const float vBase = IN.Position.y / scb.screenHeight;

    float INdepth = gBufferDepth.Sample(g_sampler, float2(uBase, vBase)).r;

    if (INdepth >= 1.0f) {
        discard;
    }

    float3 INposition = getPositionFromDepth(uBase, vBase, INdepth).xyz;
    float3 INnormal = gBufferNormal.Sample(g_sampler, float2(uBase, vBase)).xyz;

    

    return float4(uBase,vBase,0,1);
}
