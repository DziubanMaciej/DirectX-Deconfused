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

    float centerDepth = gBufferDepth.Sample(g_sampler, float2(0.5f, 0.5f)).r;

    float depthDiff = smoothstep(0.0f, 0.1f, abs(INdepth - centerDepth));

    int sampleKernelSize = max(1, int(10.0f * depthDiff));

    float3 outputColor = lightingOutput.Sample(g_sampler, float2(uBase, vBase)).rgb;

    int sampleCount = 1;

   
    const float invResWidth = 1.0f / scb.screenWidth;
    const float invResHeight = 1.0f / scb.screenHeight;

    [loop] for (int i = 1; i < sampleKernelSize; i++) {

        float tempDepth = gBufferDepth.Sample(g_sampler, float2(uBase + float(i) * invResWidth, vBase)).r;
        float depthDiff = tempDepth - centerDepth;

        if (depthDiff < 0.03f) {
            break;
        }

        outputColor += lightingOutput.Sample(g_sampler, float2(uBase + float(i) * invResWidth, vBase)).rgb;
        sampleCount++;
    }

    [loop] for (int i = -sampleKernelSize; i < -1; i++) {

        float tempDepth = gBufferDepth.Sample(g_sampler, float2(uBase + float(i) * invResWidth, vBase)).r;
        float depthDiff = tempDepth - centerDepth;

        if (depthDiff < 0.03f) {
            break;
        }

        outputColor += lightingOutput.Sample(g_sampler, float2(uBase + float(i) * invResWidth, vBase)).rgb;
        sampleCount++;
    }

    outputColor /= float(sampleCount);

    return float4(outputColor, 1);
}
