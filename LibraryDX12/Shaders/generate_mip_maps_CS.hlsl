#ifndef THREAD_GROUP_LENGTH
#define THREAD_GROUP_LENGTH 16
#endif

struct GenerateMipsCB {
    float2 texelSize;
    uint sourceMipLevel;
};
Texture2D<float4> input : register(t0);
SamplerState textureSampler : register(s0);
RWTexture2D<float4> output : register(u0);
ConstantBuffer<GenerateMipsCB> cb : register(b0);

[numthreads(THREAD_GROUP_LENGTH, THREAD_GROUP_LENGTH, 1)] void main(uint3 dispatchThreadID
                                                                    : SV_DispatchThreadID) {
    const float2 uv = (dispatchThreadID.xy + 0.5) * cb.texelSize;
    float4 pixel = input.SampleLevel(textureSampler, uv, cb.sourceMipLevel);
    output[dispatchThreadID.xy] = pixel;
}
