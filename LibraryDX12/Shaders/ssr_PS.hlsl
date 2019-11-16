
Texture2D gBufferNormal : register(t0);
Texture2D gBufferDepth : register(t1);
Texture2D gBufferSpecular : register(t2);
Texture2D lightingOutput : register(t3);

SamplerState g_sampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

struct SsrCB {
    float4 cameraPosition;
    float screenWidth;
    float screenHeight;
    matrix viewMatrixInverse;
    matrix projMatrixInverse;
    matrix viewProjMatrix;
    float4 clearColor;
};

ConstantBuffer<SsrCB> scb : register(b1);

float4 getPositionFromDepth(float u, float v, float depth) {
    float4 H = float4((u)*2 - 1, (1 - v) * 2 - 1, depth, 1.0);
    float4 D = mul(scb.projMatrixInverse, H);
    float4 INworldPosition = mul(scb.viewMatrixInverse, (D / D.w));
    return INworldPosition;
}

float4 convertToViewSpace(float4 a) {
    float4 result = mul(scb.viewProjMatrix, a);
    return result;
}

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

    float4 worldSpacePos = getPositionFromDepth(uBase, vBase, INdepth);

    float3 cameraDirection = worldSpacePos.xyz - scb.cameraPosition.xyz;

    float3 INnormal = normalize(gBufferNormal.Sample(g_sampler, float2(uBase, vBase)).xyz);

    float3 reflectDir = normalize(reflect(cameraDirection, INnormal));

    float4 rayPoint = worldSpacePos;

    float marchingStep = distance(scb.cameraPosition.xyz, worldSpacePos.xyz) / 50.0f;

    int marchQty = min(int(10.0f / marchingStep), 100);

    [loop] for (int i = 0; i < marchQty; i++) {

        rayPoint.xyz += reflectDir * marchingStep;

        float4 screenSpacePos = convertToViewSpace(rayPoint);
        screenSpacePos.x = screenSpacePos.x / screenSpacePos.w * 0.5f + 0.5f;
        screenSpacePos.y = -screenSpacePos.y / screenSpacePos.w * 0.5f + 0.5f;

        // Depth test
        float pointDepth = gBufferDepth.SampleLevel(g_sampler, screenSpacePos.xy, 0).r;

        if ((screenSpacePos.z / screenSpacePos.w) >= pointDepth) {

            // Too far Depth/Distance test
            float3 posFromDepth = getPositionFromDepth(screenSpacePos.x, screenSpacePos.y, pointDepth).xyz;
            float rayDistance = distance(posFromDepth.xyz, rayPoint.xyz);
            if (rayDistance > (2.0f * marchingStep)) {
                continue;
            }

            // Screen test
            if (screenSpacePos.x < 0.0f || screenSpacePos.x > 1.0f) {
                return float4(scb.clearColor.xyz, 1.0f);
            }
            if (screenSpacePos.y < 0.0f || screenSpacePos.y > 1.0f) {
                return float4(scb.clearColor.xyz, 1.0f);
            }

            float screenEdgefactor = smoothstep(0.0f, 0.4f, 1.0f - (distance(float2(0.5f, 0.5f), screenSpacePos.xy) * 2.0f));

            float3 OUTssr = screenEdgefactor * lightingOutput.SampleLevel(g_sampler, screenSpacePos.xy, 0).xyz + scb.clearColor.xyz * (1.0f - screenEdgefactor);

            return float4(OUTssr, 1.0f);
        }
    }

    return float4(scb.clearColor.xyz, 1.0f);
}