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
    
    float4 H = float4((u) * 2 - 1, (1 - v) * 2 - 1, depth, 1.0);
    float4 D = mul(scb.projMatrixInverse, H);
    float4 INworldPosition = mul(scb.viewMatrixInverse, (D / D.w));

    return INworldPosition;
}

float2 getRandom(float u, float v) {
    float2 t = float2(u / v + u, v*u + v);
    t.x = t.x + 0.707f;
    t.y = t.y + 0.407f;
    t.x = t.x * 3;
    t.y = t.y * 4;
    return normalize(t * 2.0f - 1.0f);
}

float doAmbientOcclusion(float2 tcoord, float2 uv, float3 pos, float3 norm)
{
    float depth = gBufferDepth.Sample(g_sampler, tcoord + uv).r;
    float3 diff = getPositionFromDepth(tcoord.x + uv.x, tcoord.y + uv.y, depth).xyz - pos;
    const float3 v = normalize(diff);
    const float d = length(diff) * 2.0f;
    return max(0.0, dot(norm, v) - 0.01f)*(1.0f / (1.0f + d)) * 3.0f;
}

float4 main(PixelShaderInput IN) : SV_Target {

    const float uBase = IN.Position.x / scb.screenWidth;
    const float vBase = IN.Position.y / scb.screenHeight;

    float INdepth = gBufferDepth.Sample(g_sampler, float2(uBase, vBase)).r;

    if (INdepth >= 1) {
        discard;
    }

    const float2 vec[4] = { float2(1,0), float2(-1,0), float2(0,1), float2(0,-1) };
    float3 INposition = getPositionFromDepth(uBase, vBase, INdepth).xyz;
    float3 INnormal = gBufferNormal.Sample(g_sampler, float2(uBase, vBase)).xyz;
    //float2 rand = getRandom(uBase, vBase);
    float2 rand = float2(0.0f, 0.0f);

    float ao = 0.0f;
    float rad = 4.0f / INdepth;
    //float rad = 1.0f / scb.screenWidth;

    for (int i = -3; i < 4; i++) {
        for (int j = -3; j < 4; j++) {
            //float2 coord1 = reflect(vec[i], rand) * rad;
            //float2 coord2 = float2(coord1.x * 0.707f - coord1.y * 0.707f, coord1.x * 0.707f + coord1.y * 0.707f);

            //ao = ao + doAmbientOcclusion(float2(uBase, vBase), coord1 * 0.25, INposition, INnormal, INdepth);
            //ao = ao + doAmbientOcclusion(float2(uBase, vBase), coord2 * 0.5, INposition, INnormal, INdepth);
            //ao = ao + doAmbientOcclusion(float2(uBase, vBase), coord1 * 0.75, INposition, INnormal, INdepth);
            //ao = ao + doAmbientOcclusion(float2(uBase, vBase), coord2, INposition, INnormal, INdepth);

            ao = ao + doAmbientOcclusion(float2(uBase, vBase), float2((i * rad) / scb.screenWidth, float(j * rad) / scb.screenHeight), INposition, normalize(INnormal));
            //ao = ao + doAmbientOcclusion(float2(uBase, vBase), float2(0.0f / scb.screenWidth, float(i*4) / scb.screenHeight), INposition, INnormal, INdepth);
            //ao = ao + doAmbientOcclusion(float2(uBase, vBase), float2(4.0f / scb.screenWidth, float(i*4) / scb.screenHeight), INposition, INnormal, INdepth);
            //ao = ao + doAmbientOcclusion(float2(uBase, vBase), float2(8.0f / scb.screenWidth, float(i*4) / scb.screenHeight), INposition, INnormal, INdepth);
        }
    }

    ao = pow(ao / 49.0f, 2.0f);

    ao = 1 - (ao * 0.8f);

    return float4(ao, ao, ao, 1);
}
