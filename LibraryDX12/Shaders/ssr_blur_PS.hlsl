
Texture2D gBufferSpecular : register(t0);
Texture2D gBufferDepth : register(t1);
Texture2D ssrOutput : register(t2);

SamplerState g_sampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

struct SsrMergeCB {
    float screenWidth;
    float screenHeight;
};

ConstantBuffer<SsrMergeCB> scb : register(b1);

float4 main(PixelShaderInput IN) : SV_Target {

    const float uBase = IN.Position.x / scb.screenWidth;
    const float vBase = IN.Position.y / scb.screenHeight;

    float INdepth = gBufferDepth.Sample(g_sampler, float2(uBase, vBase)).r;

    if (INdepth >= 1.0f) {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    float INspecularity = gBufferSpecular.Sample(g_sampler, float2(uBase, vBase)).r;

    if (INspecularity <= 0.0f) {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    const float uSsrInv = 1.0f / (scb.screenWidth / 2);
    const float vSsrInv = 1.0f / (scb.screenHeight / 2);

    float3 INssrOutput = ssrOutput.Sample(g_sampler, float2(uBase, vBase)).xyz;

    int samplesCount = 1;

    // Vertical
    for (int i = 1; i <= 4; i++) {
        float tempDepth = gBufferDepth.Sample(g_sampler, float2(uBase, vBase + float(i) * vSsrInv)).r;

        float depthDiff = abs(tempDepth - INdepth) / INdepth;

        if (depthDiff > 0.001f) {
            break;
        }

        samplesCount += 1;

        INssrOutput += ssrOutput.Sample(g_sampler, float2(uBase, vBase + float(i) * vSsrInv)).xyz;
    }

    for (int i = -1; i >= -4; i--) {
        float tempDepth = gBufferDepth.Sample(g_sampler, float2(uBase, vBase + float(i) * vSsrInv)).r;

        float depthDiff = abs(tempDepth - INdepth) / INdepth;

        if (depthDiff > 0.001f) {
            break;
        }

        samplesCount += 1;
        INssrOutput += ssrOutput.Sample(g_sampler, float2(uBase, vBase + float(i) * vSsrInv)).xyz;
    }

    INssrOutput /= float(samplesCount);

    
    /*INssrOutput += ssrOutput.Sample(g_sampler, float2(uBase + 1.0f / scb.screenWidth, vBase + 1.0f / scb.screenHeight)).xyz;
    INssrOutput += ssrOutput.Sample(g_sampler, float2(uBase + 1.0f / scb.screenWidth, vBase - 1.0f / scb.screenHeight)).xyz;
    INssrOutput += ssrOutput.Sample(g_sampler, float2(uBase - 1.0f / scb.screenWidth, vBase + 1.0f / scb.screenHeight)).xyz;
    INssrOutput += ssrOutput.Sample(g_sampler, float2(uBase - 1.0f / scb.screenWidth, vBase - 1.0f / scb.screenHeight)).xyz;

    INssrOutput /= 5.0f;*/

    //float3 INssrOutput = ssrOutput.Sample(g_sampler, float2(uBase, vBase)).xyz;

    //float screenEdgefactor = smoothstep(0.0f, 0.4f, 1.0f - (distance(float2(0.5f, 0.5f), screenSpacePos.xy) * 2.0f));

    //float3 OUTssr = screenEdgefactor * lightingOutput.SampleLevel(g_sampler, screenSpacePos.xy, 0).xyz + (1.0f - screenEdgefactor) * scb.clearColor.xyz;

    //return float4(((INlightingOutput * (1 - ssrPower)) + (ssrPower * OUTssr)), 1.0f);



    return float4(INssrOutput.xyz, 1.0f);
}