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

    float3 outputColor = float3(0.0f, 0.0f, 0.0f);

    [loop] for (int i = -sampleKernelSize; i <= sampleKernelSize; i++) {
        [loop] for (int j = -sampleKernelSize; j <= sampleKernelSize; j++) {
            float invResWidth = 1.0f / scb.screenWidth;
            float invResHeight = 1.0f / scb.screenHeight;
            outputColor += lightingOutput.Sample(g_sampler, float2(uBase + float(i) * invResWidth, vBase + float(j) * invResHeight)).rgb;
        }
    }

    outputColor /= float(pow((2.0f * sampleKernelSize + 1.0f), 2.0f));

    return float4(outputColor, 1);
}
