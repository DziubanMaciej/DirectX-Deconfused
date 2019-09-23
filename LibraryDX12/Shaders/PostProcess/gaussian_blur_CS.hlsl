// -------------------------------------------------------- Parameters (defines)
// HORIZONTAL - defined for horizontal pass, not defined for a vertical pass
// SAMPLING_BORDER - how many pixels in each direction will be sampled to blur
// THREAD_GROUP_LENGTH - how many threads in a wave in one dimension

#ifndef SAMPLING_BORDER
#define SAMPLING_BORDER 5
#endif

#ifndef THREAD_GROUP_LENGTH
#define THREAD_GROUP_LENGTH 16
#endif

// -------------------------------------------------------- Contants
static const float weights[] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};
#define SAMPLED_PIXELS_SHORT_DIMENSION (THREAD_GROUP_LENGTH)                      // one dimension is not changed
#define SAMPLED_PIXELS_LONG_DIMENSION (THREAD_GROUP_LENGTH + 2 * SAMPLING_BORDER) // we have a border in second dimension
#define SAMPLED_PIXELS_COUNT (SAMPLED_PIXELS_SHORT_DIMENSION * SAMPLED_PIXELS_LONG_DIMENSION)

// -------------------------------------------------------- Horizontal/Vertical abstraction

#ifdef HORIZONTAL
#define FLATTEN_GROUP_MEMORY_INDEX(index) ((index.y) * SAMPLED_PIXELS_LONG_DIMENSION + (index.x))
#define OFFSET_UV(base, offset) (float2(base.x + offset.x, base.y))
#define OFFSET_GROUP_MEMORY_INDEX(base, offset) (uint2(base.x + (offset), base.y))
#define SELECT_COMPONENT(GroupThreadID) (GroupThreadID.x)
#else
#define FLATTEN_GROUP_MEMORY_INDEX(index) ((index.y) * SAMPLED_PIXELS_SHORT_DIMENSION + (index.x))
#define OFFSET_UV(base, offset) (float2(base.x, base.y + offset.y))
#define OFFSET_GROUP_MEMORY_INDEX(base, offset) (uint2(base.x, base.y + (offset)))
#define SELECT_COMPONENT(GroupThreadID) (GroupThreadID.y)
#endif

// -------------------------------------------------------- Shader input

struct PostProcessGaussianBlurComputeCB {
    float screenWidth;
    float screenHeight;
};
Texture2D input : register(t0);
SamplerState sceneSampler : register(s0);
RWTexture2D<float4> output : register(u0);
ConstantBuffer<PostProcessGaussianBlurComputeCB> cb : register(b0);

struct ComputeShaderInput {
    uint3 GroupID : SV_GroupID;                   // 3D index of the thread group in the dispatch.
    uint3 GroupThreadID : SV_GroupThreadID;       // 3D index of local thread ID in a thread group.
    uint3 DispatchThreadID : SV_DispatchThreadID; // 3D index of global thread ID in the dispatch.
    uint GroupIndex : SV_GroupIndex;              // Flattened local index of the thread within a thread group.
};

// -------------------------------------------------------- Group Memory

groupshared float groupMemoryR[SAMPLED_PIXELS_COUNT];
groupshared float groupMemoryG[SAMPLED_PIXELS_COUNT];
groupshared float groupMemoryB[SAMPLED_PIXELS_COUNT];

void storeInGroupMemory(uint2 index, float3 pixel) {
    const uint flatIndex = FLATTEN_GROUP_MEMORY_INDEX(index);
    groupMemoryR[flatIndex] = pixel.r;
    groupMemoryG[flatIndex] = pixel.g;
    groupMemoryB[flatIndex] = pixel.b;
}

float3 loadFromGroupMemory(uint2 index) {
    const uint flatIndex = FLATTEN_GROUP_MEMORY_INDEX(index);
    return float3(groupMemoryR[flatIndex], groupMemoryG[flatIndex], groupMemoryB[flatIndex]);
}

// -------------------------------------------------------- Compute shader

void sampleBorderPixels(uint3 GroupThreadID, float2 uvBase, float2 uvOffset, uint2 groupMemoryIndexBase) {
    if (SELECT_COMPONENT(GroupThreadID) < SAMPLING_BORDER) {
        const float2 uv = OFFSET_UV(uvBase, -SAMPLING_BORDER * uvOffset);
        const float2 groupMemoryIndex = OFFSET_GROUP_MEMORY_INDEX(groupMemoryIndexBase, -SAMPLING_BORDER);
        const float3 borderPixel = input.SampleLevel(sceneSampler, uv, 0).rgb;
        storeInGroupMemory(groupMemoryIndex, borderPixel);
    }

    if (SELECT_COMPONENT(GroupThreadID) >= THREAD_GROUP_LENGTH - SAMPLING_BORDER) {
        const float2 uv = OFFSET_UV(uvBase, SAMPLING_BORDER * uvOffset);
        const float2 groupMemoryIndex = OFFSET_GROUP_MEMORY_INDEX(groupMemoryIndexBase, SAMPLING_BORDER);
        const float3 borderPixel = input.SampleLevel(sceneSampler, uv, 0).rgb;
        storeInGroupMemory(groupMemoryIndex, borderPixel);
    }
}

[numthreads(THREAD_GROUP_LENGTH, THREAD_GROUP_LENGTH, 1)] void main(ComputeShaderInput IN) {
    // Constants
    const uint2 groupMemoryIndexBase = OFFSET_GROUP_MEMORY_INDEX(IN.GroupThreadID, SAMPLING_BORDER);
    const float2 uvBase = float2(IN.DispatchThreadID.x / cb.screenWidth, IN.DispatchThreadID.y / cb.screenHeight);
    const float2 uvOffset = float2(1 / cb.screenWidth, 1 / cb.screenHeight);

    // Sample pixel for this thread and store it in groupshared memory
    const float4 pixel = input.SampleLevel(sceneSampler, uvBase, 0);
    storeInGroupMemory(groupMemoryIndexBase, pixel);

    // Sample border pixels
    sampleBorderPixels(IN.GroupThreadID, uvBase, uvOffset, groupMemoryIndexBase);

    // Group memory barrier to ensure all writes are done
    GroupMemoryBarrierWithGroupSync();

    // Blur
    float4 resultColor = pixel;
    resultColor.rgb = loadFromGroupMemory(groupMemoryIndexBase);
    resultColor.rgb *= weights[0];
    for (int i = 1; i < SAMPLING_BORDER; i++) {
        resultColor.rgb += weights[i] * loadFromGroupMemory(OFFSET_GROUP_MEMORY_INDEX(groupMemoryIndexBase, i));
        resultColor.rgb += weights[i] * loadFromGroupMemory(OFFSET_GROUP_MEMORY_INDEX(groupMemoryIndexBase, -i));
    }

    // Write result to the UAV
    output[IN.DispatchThreadID.xy] = resultColor;
}
