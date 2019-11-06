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
    return max(0.0, dot(norm, v) - 0.2f) * (1.0f / (1.0f + d)) * 1.5f;
}

#define DISK_SAMPLE_COUNT (16)
static float2 poissonDisk[16] = {
    float2(0.2770745f, 0.6951455f),
    float2(0.1874257f, -0.02561589f),
    float2(-0.3381929f, 0.8713168f),
    float2(0.5867746f, 0.1087471f),
    float2(-0.3078699f, 0.188545f),
    float2(0.7993396f, 0.4595091f),
    float2(-0.09242552f, 0.5260149f),
    float2(0.3657553f, -0.5329605f),
    float2(-0.3829718f, -0.2476171f),
    float2(-0.01085108f, -0.6966301f),
    float2(0.8404155f, -0.3543923f),
    float2(-0.5186161f, -0.7624033f),
    float2(-0.8135794f, 0.2328489f),
    float2(-0.784665f, -0.2434929f),
    float2(0.9920505f, 0.0855163f),
    float2(-0.687256f, 0.6711345f)};

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

    int sampleCount = DISK_SAMPLE_COUNT;
    int sampleRadius = 8.0f;

    for (int i = 0; i < sampleCount; i++) {
        float2 pos = (poissonDisk[i].xy / float2(scb.screenWidth, scb.screenHeight)) * sampleRadius;
        ao = ao + doAmbientOcclusion(float2(uBase, vBase), pos, INposition, normalize(INnormal));
    }

    ao = 1.0f - abs(ao / float(sampleCount));

    return ao;
}
