cbuffer SimpleConstantBuffer : register(b0) {
    float4 cameraPosition;
    float3 ambientLight;
    float _padding;
    float shadowMapSize;
    float screenWidth;
    float screenHeight;
    int lightsSize;
    float4 lightPosition[8];
    float4 lightColor[8];
    float4 lightDirection[8];
    matrix smVpMatrix[8];
};

struct InverseViewProj {
    matrix viewMatrixInverse;
    matrix projMatrixInverse;
    bool enableSSAO;
};

ConstantBuffer<InverseViewProj> invVP : register(b1);

Texture2D gBufferAlbedo : register(t0);
Texture2D gBufferNormal : register(t1);
Texture2D gBufferSpecular : register(t2);
Texture2D gBufferDepth : register(t3);
Texture2D ssaoMap : register(t4);
Texture2D shadowMaps[8] : register(t5);

SamplerState g_sampler : register(s0);
SamplerState g_sampler_bilinear : register(s1);
SamplerState g_sampler_sm : register(s2);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

struct PS_OUT {
    float4 bloomMap;
    float4 lightingOutput;
};

static float2 poissonDisk[16] = {
    float2(0.2770745f, 0.6951455f),
    float2(0.1874257f, -0.02561589f),
    float2(-0.3381929f, 0.8713168f),
    float2(0.5867746f, 0.1087471f),
    float2(-0.3078699f, 0.188545f),
    float2(0.7993396f, 0.4595091f),
    float2(-0.09242552f, 0.5260149f),
    float2(0.3657553f, -0.5329605f),
    float2(-0.3829718f, -0.2476171f),
    float2(-0.01085108f, -0.6966301f),
    float2(0.8404155f, -0.3543923f),
    float2(-0.5186161f, -0.7624033f),
    float2(-0.8135794f, 0.2328489f),
    float2(-0.784665f, -0.2434929f),
    float2(0.9920505f, 0.0855163f),
    float2(-0.687256f, 0.6711345f)};

float calculateShadowFactor(float4 smCoords, int shadowMapIndex) {
    if (shadowMapSize == 0.f) {
        return 1.f;
    }

    if (shadowMapSize > 2000.0f) {
        float shadowFactor = 16;
        float smDepth = 0;

        if (smCoords.x >= 0 && smCoords.x <= 1) {
            if (smCoords.y >= 0 && smCoords.y <= 1) {

                for (int oxy = 0; oxy < 16; oxy++) {
                    float2 offset = (poissonDisk[oxy] * 2.0f) / float2(shadowMapSize, shadowMapSize);
                    smDepth = shadowMaps[shadowMapIndex].SampleLevel(g_sampler_sm, smCoords.xy + offset, 0).r;

                    if (((smCoords.z / smCoords.w) - 0.001f) > smDepth) {
                        shadowFactor = shadowFactor - 1;
                    }
                }
            }
        }
        shadowFactor = shadowFactor / 16;
        return shadowFactor;
    } else {
        float shadowFactor = 9;
        float smDepth = 0;

        if (smCoords.x >= 0 && smCoords.x <= 1) {
            if (smCoords.y >= 0 && smCoords.y <= 1) {

                for (int ox = -1; ox <= 1; ox++) {
                    for (int oy = -1; oy <= 1; oy++) {
                        float2 offset = float2(ox, oy) / float2(shadowMapSize, shadowMapSize);
                        smDepth = shadowMaps[shadowMapIndex].SampleLevel(g_sampler_sm, smCoords.xy + offset, 0).r;

                        if (((smCoords.z / smCoords.w) - 0.003f) > smDepth) {
                            shadowFactor = shadowFactor - 1;
                        }
                    }
                }
            }
        }
        shadowFactor = shadowFactor / 9;
        return shadowFactor;
    }
}

PS_OUT main(PixelShaderInput IN) : SV_Target {
    const float uBase = IN.Position.x / screenWidth;
    const float vBase = IN.Position.y / screenHeight;

    // Depth test
    const float INdepth = gBufferDepth.Sample(g_sampler, float2(uBase, vBase)).r;
    if (INdepth >= 1) {
        discard;
    }

    // World position
    const float4 H = float4((uBase)*2 - 1, (1 - vBase) * 2 - 1, INdepth, 1.0);
    const float4 D = mul(invVP.projMatrixInverse, H);
    const float4 INworldPosition = mul(invVP.viewMatrixInverse, (D / D.w));

    // Sample gbuffers
    const float4 INnormal = gBufferNormal.Sample(g_sampler, float2(uBase, vBase));
    const float2 INspecularity = gBufferSpecular.Sample(g_sampler, float2(uBase, vBase)).xy;
    const float4 INalbedo = gBufferAlbedo.Sample(g_sampler, float2(uBase, vBase));

    // Result
    float4 OUT_Color = float4(0, 0, 0, 1);

    for (int i = 0; i < lightsSize; i++) {

        // Skip light if power is 0
        if (lightColor[i].w == 0.0f) {
            continue;
        }

        //Check for shadow
        float4 smCoords = (mul(smVpMatrix[i], INworldPosition)).xyzw;
        smCoords.x = smCoords.x / smCoords.w / 2.0f + 0.5f;
        smCoords.y = -smCoords.y / smCoords.w / 2.0f + 0.5f;
        const float shadowFactor = calculateShadowFactor(smCoords, i);
        if (shadowFactor == 0.0f) {
            continue;
        }

        //Light color
        float3 tempLightColor = lightColor[i].xyz / 3;

        //Diffuse
        float lightDistance = distance(INworldPosition.xyz, lightPosition[i].xyz);
        float tempLightPower = (30 / (lightDistance * lightDistance)) * lightColor[i].w;

        //Normal
        float3 lightPositionNorm = normalize(lightPosition[i].xyz - INworldPosition.xyz);
        float normalPower = max(dot(INnormal.xyz, lightPositionNorm), 0);

        //Specular
        float3 viewDir = normalize(cameraPosition.xyz - INworldPosition.xyz);
        float3 reflectDir = reflect(-lightPositionNorm, INnormal.xyz);
        float specularPower = pow(max(dot(viewDir, reflectDir), 0.0), 36) * INspecularity.x;

        //Direction
        float3 lightDirNorm = normalize(lightDirection[i].xyz);
        float directionPower = pow(max((dot(-lightPositionNorm.xyz, lightDirNorm.xyz)), 0.0), 4);

        OUT_Color.xyz += ((tempLightColor.xyz * (1.0f - INspecularity.x)) + INalbedo.xyz) * tempLightPower * (normalPower + specularPower) * directionPower * shadowFactor;
    }

    // Ambient lighting
    const float3 ambientLightColor = ambientLight.rgb / 4;
    const float ambientLightPower = length(ambientLight.rgb) * 0.5f;
    OUT_Color.xyz += ((INalbedo.xyz + (ambientLightColor * (1.0f - INspecularity.x))) * ambientLightPower * (INspecularity.x / 10.0f + 1.0f));

    PS_OUT result;

    // Shader output - regular scene
    if (invVP.enableSSAO) {
        const float INssao = ssaoMap.Sample(g_sampler_bilinear, float2(uBase, vBase)).r;
        result.lightingOutput = OUT_Color * INssao;
    } else {
        result.lightingOutput = OUT_Color;
    }

    // Shader output - bloom map
    //const float brightness = dot(OUT_Color.rgb, float3(0.2126f, 0.7152f, 0.0722f));
    if (INspecularity.g > 0.0f) {
        result.bloomMap = float4(INalbedo.rgb * INspecularity.g, INspecularity.g);
    } else {
        result.bloomMap = float4(0, 0, 0, 0);
    }

    return result;
}
