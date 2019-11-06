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

float doAmbientOcclusion(float2 tcoord, float2 uv, float3 pos, float3 norm, float bias) {
    float depth = gBufferDepth.Sample(g_sampler, tcoord + uv).r;
    float3 diff = getPositionFromDepth(tcoord.x + uv.x, tcoord.y + uv.y, depth).xyz - pos;
    const float3 v = normalize(diff);
    const float d = length(diff);
    return max(0.0, dot(norm, v) - bias) * (1.0f / (1.0f + sqrt(d))) * 2.0f;
}

//Source: https://github.com/GameTechDev/ASSAO/blob/master/Projects/ASSAO/ASSAO/ASSAO.hlsl
#define INTELSSAO_MAIN_DISK_SAMPLE_COUNT (32)
static const float4 g_samplePatternMain[INTELSSAO_MAIN_DISK_SAMPLE_COUNT] ={
    0.78488064, 0.56661671, 1.500000, -0.126083, 0.26022232, -0.29575172, 1.500000, -1.064030, 0.10459357, 0.08372527, 1.110000, -2.730563, -0.68286800, 0.04963045, 1.090000, -0.498827,
    -0.13570161, -0.64190155, 1.250000, -0.532765, -0.26193795, -0.08205118, 0.670000, -1.783245, -0.61177456, 0.66664219, 0.710000, -0.044234, 0.43675563, 0.25119025, 0.610000, -1.167283,
    0.07884444, 0.86618668, 0.640000, -0.459002, -0.12790935, -0.29869005, 0.600000, -1.729424, -0.04031125, 0.02413622, 0.600000, -4.792042, 0.16201244, -0.52851415, 0.790000, -1.067055,
    -0.70991218, 0.47301072, 0.640000, -0.335236, 0.03277707, -0.22349690, 0.600000, -1.982384, 0.68921727, 0.36800742, 0.630000, -0.266718, 0.29251814, 0.37775412, 0.610000, -1.422520,
    -0.12224089, 0.96582592, 0.600000, -0.426142, 0.11071457, -0.16131058, 0.600000, -2.165947, 0.46562141, -0.59747696, 0.600000, -0.189760, -0.51548797, 0.11804193, 0.600000, -1.246800,
    0.89141309, -0.42090443, 0.600000, 0.028192, -0.32402530, -0.01591529, 0.600000, -1.543018, 0.60771245, 0.41635221, 0.600000, -0.605411, 0.02379565, -0.08239821, 0.600000, -3.809046,
    0.48951152, -0.23657045, 0.600000, -1.189011, -0.17611565, -0.81696892, 0.600000, -0.513724, -0.33930185, -0.20732205, 0.600000, -1.698047, -0.91974425, 0.05403209, 0.600000, 0.062246,
    -0.15064627, -0.14949332, 0.600000, -1.896062, 0.53180975, -0.35210401, 0.600000, -0.758838, 0.41487166, 0.81442589, 0.600000, -0.505648, -0.24106961, -0.32721516, 0.600000, -1.665244
};

float2 main(PixelShaderInput IN) : SV_Target {

    const float uBase = IN.Position.x / scb.screenWidth;
    const float vBase = IN.Position.y / scb.screenHeight;

    float INdepth = gBufferDepth.Sample(g_sampler, float2(uBase, vBase)).r;

    if (INdepth >= 1.0f) {
        discard;
    }

    float3 INposition = getPositionFromDepth(uBase, vBase, INdepth).xyz;
    float3 INnormal = gBufferNormal.Sample(g_sampler, float2(uBase, vBase)).xyz;

    float ao = 0.0f;

    int sampleCount = 32;
    int sampleRadius = 8.0f;
    float ssaoBias = 0.2f;

    if (INdepth >= 0.99f) {
        sampleCount = 16;
    }

    for (int i = 0; i < sampleCount; i++) {
        float2 pos = (g_samplePatternMain[i] / float2(scb.screenWidth, scb.screenHeight)) * sampleRadius;
        ao = ao + doAmbientOcclusion(float2(uBase, vBase), pos, INposition, normalize(INnormal), ssaoBias);
    }

    ao = 1.0f - abs(ao / float(sampleCount));

    return float2(ao, INdepth);
}
