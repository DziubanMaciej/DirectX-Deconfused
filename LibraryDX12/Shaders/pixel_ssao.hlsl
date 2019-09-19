Texture2D gBufferNormal : register(t0);
Texture2D gBufferDepth : register(t1);

SamplerState g_sampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

struct SsaoCB {
    float screenWidth;
    float screenHeight;
    matrix viewMatrixInverse;
    matrix projMatrixInverse;
};

ConstantBuffer<SsaoCB> scb : register(b1);

float4 getPositionFromDepth(float u, float v, float depth) {
    float4 H = float4((u)*2 - 1, (1 - v) * 2 - 1, depth, 1.0);
    float4 D = mul(scb.projMatrixInverse, H);
    float4 INworldPosition = mul(scb.viewMatrixInverse, (D / D.w));
    return INworldPosition;
}

float doAmbientOcclusion(float2 tcoord, float2 uv, float3 pos, float3 norm) {
    float depth = gBufferDepth.Sample(g_sampler, tcoord + uv).r;
    float3 diff = getPositionFromDepth(tcoord.x + uv.x, tcoord.y + uv.y, depth).xyz - pos;
    const float3 v = normalize(diff);
    const float d = length(diff) * 2.0f;
    return max(0.0, dot(norm, v) - 0.2f) * (1.0f / (1.0f + d)) * 3.0f;
}

float main(PixelShaderInput IN) : SV_Target {

    const float uBase = IN.Position.x / scb.screenWidth;
    const float vBase = IN.Position.y / scb.screenHeight;

    float INdepth = gBufferDepth.Sample(g_sampler, float2(uBase, vBase)).r;

    if (INdepth >= 1.0f) {
        discard;
    }

    float3 INposition = getPositionFromDepth(uBase, vBase, INdepth).xyz;
    float3 INnormal = gBufferNormal.Sample(g_sampler, float2(uBase, vBase)).xyz;

    float ao = 0.0f;
    float rad = 4.0f / INdepth;

    for (int i = -2; i < 3; i++) {
        for (int j = -2; j < 3; j++) {
            ao = ao + doAmbientOcclusion(float2(uBase, vBase), float2((i * rad) / scb.screenWidth, float(j * rad) / scb.screenHeight), INposition, normalize(INnormal));
        }
    }

    ao = abs(ao / 25.0f);
    ao = pow(ao, 1.2f);
    ao = 1 - ao;

    return ao;
}
