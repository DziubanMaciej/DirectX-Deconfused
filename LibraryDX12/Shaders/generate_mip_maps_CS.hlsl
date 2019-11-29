#ifndef THREAD_GROUP_LENGTH
#define THREAD_GROUP_LENGTH 16
#endif

struct GenerateMipsCB {
    float2 texelSize;
    uint sourceMipLevel;
    bool isSrgb;
};
Texture2D<float4> input : register(t0);
SamplerState textureSampler : register(s0);
RWTexture2D<float4> output : register(u0);
ConstantBuffer<GenerateMipsCB> cb : register(b0);

float3 linearToSRGB(float3 x) {
    return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(abs(x - 0.00228)) - 0.13448 * x + 0.005719;
}

[numthreads(THREAD_GROUP_LENGTH, THREAD_GROUP_LENGTH, 1)] void main(uint3 dispatchThreadID
                                                                    : SV_DispatchThreadID) {
    const float2 uv = (dispatchThreadID.xy + 0.5) * cb.texelSize;
    float4 pixel = input.SampleLevel(textureSampler, uv, cb.sourceMipLevel);

    // SRVs (input) can be SRGB textures, but UAVs (ouput) cannot, so format for UAV had to be artificially converted
    // to linear. To output the correct collor, we have to do the transformation manually in the shader.
    if (cb.isSrgb) {
        pixel.rgb = linearToSRGB(pixel.rgb);
    }
    output[dispatchThreadID.xy] = pixel;
}
