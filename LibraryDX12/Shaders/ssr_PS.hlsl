
Texture2D gBufferNormal : register(t0);
Texture2D gBufferDepth : register(t1);
Texture2D gBufferSpecular : register(t2);
Texture2D prevFrame : register(t3);

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

float3 hash(float3 a) {
    a = frac(a * float3(0.8f, 0.8f, 0.8f));
    a += dot(a, a.yxz + 19.19f);
    return frac((a.xxy + a.yxx) * a.zyx);
}

float4 main(PixelShaderInput IN) : SV_Target {

    const float uBase = IN.Position.x / scb.screenWidth;
    const float vBase = IN.Position.y / scb.screenHeight;

    float INdepth = gBufferDepth.Sample(g_sampler, float2(uBase, vBase)).r;

    if (INdepth >= 1.0f) {
        discard;
    }

    float INspecularity = gBufferSpecular.Sample(g_sampler, float2(uBase, vBase)).r;

    if (INspecularity <= 0.0f) {
        discard;
    }

    float4 worldSpacePos = getPositionFromDepth(uBase, vBase, INdepth);

    float3 cameraDirection = worldSpacePos.xyz - scb.cameraPosition;

    float3 INnormal = normalize(gBufferNormal.Sample(g_sampler, float2(uBase, vBase)).xyz);

    float3 jitt = lerp(float3(0.0f, 0.0f, 0.0f), hash(worldSpacePos.xyz), INspecularity) * 0.05f;

    float3 reflectDir = normalize(reflect(cameraDirection, INnormal)) + jitt;

    float4 rayPoint = worldSpacePos;

    [loop]
    for (int i = 0; i < 75; i++) {

        float marchingStep = 0.1f;

        rayPoint.xyz += reflectDir * marchingStep;

        float4 screenSpacePos = convertToViewSpace(rayPoint);
        screenSpacePos.x = screenSpacePos.x / screenSpacePos.w * 0.5f + 0.5f;
        screenSpacePos.y = -screenSpacePos.y / screenSpacePos.w * 0.5f + 0.5f;

        // Depth test
        float pointDepth = gBufferDepth.Sample(g_sampler, screenSpacePos.xy).r;

        float deltaDepth = (screenSpacePos.z / screenSpacePos.w) - pointDepth;

        if (deltaDepth >= 0.0f) {

            // Too far Depth/Distance test
            float3 posFromDepth = getPositionFromDepth(screenSpacePos.x, screenSpacePos.y, pointDepth).xyz;
            float rayDistance = distance(posFromDepth, rayPoint);
            if (rayDistance > 0.3f) {
                continue;
            }

            // Screen test
            if (screenSpacePos.x < 0.0f || screenSpacePos.x > 1.0f) {
                discard;
            }
            if (screenSpacePos.y < 0.0f || screenSpacePos.y > 1.0f) {
                discard;
            }

            // Binary Search
            bool lastHit = true;

            for (int i = 0; i < 10; i++) {

                if (lastHit == true) {
                    marchingStep = -(marchingStep / 2.0f);
                } else {
                    marchingStep = (marchingStep / 2.0f);
                }

                rayPoint.xyz += reflectDir * marchingStep;

                screenSpacePos = convertToViewSpace(rayPoint);
                screenSpacePos.x = screenSpacePos.x / screenSpacePos.w * 0.5f + 0.5f;
                screenSpacePos.y = -screenSpacePos.y / screenSpacePos.w * 0.5f + 0.5f;

                deltaDepth = (screenSpacePos.z / screenSpacePos.w) - pointDepth;

                if (deltaDepth >= 0.0f) {
                    lastHit = true;
                } else {
                    lastHit = false;
                }
            }

            return float4(prevFrame.Sample(g_sampler, screenSpacePos.xy).xyz, 1.0f);
        }
    }

    discard;
    return float4(0, 0, 0, 0);
}