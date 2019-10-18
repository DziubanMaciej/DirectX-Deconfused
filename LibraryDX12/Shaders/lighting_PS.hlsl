cbuffer SimpleConstantBuffer : register(b0) {
    float4 cameraPosition;
    int lightsSize;
    float3 ambientLight;
    float4 lightPosition[8];
    float4 lightColor[8];
    float4 lightDirection[8];
    matrix smVpMatrix[8];
    float screenWidth;
    float screenHeight;
};

struct InverseViewProj {
    matrix viewMatrixInverse;
    matrix projMatrixInverse;
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

PS_OUT main(PixelShaderInput IN) : SV_Target {

    const float uBase = IN.Position.x / screenWidth;
    const float vBase = IN.Position.y / screenHeight;

    float INdepth = gBufferDepth.Sample(g_sampler, float2(uBase, vBase)).r;

    if (INdepth >= 1) {
        discard;
    }

    float4 H = float4((uBase)*2 - 1, (1 - vBase) * 2 - 1, INdepth, 1.0);
    float4 D = mul(invVP.projMatrixInverse, H);
    float4 INworldPosition = mul(invVP.viewMatrixInverse, (D / D.w));

    float4 INnormal = gBufferNormal.Sample(g_sampler, float2(uBase, vBase));
    float2 INspecularity = gBufferSpecular.Sample(g_sampler, float2(uBase, vBase));
    float4 INalbedo = gBufferAlbedo.Sample(g_sampler, float2(uBase, vBase));
    float INssao = ssaoMap.Sample(g_sampler_bilinear, float2(uBase, vBase)).r;

    float4 OUT_Color = float4(0, 0, 0, 1);

    for (int i = 0; i < lightsSize; i++) {

        //Check for shadow
        float4 smCoords = (mul(smVpMatrix[i], INworldPosition)).xyzw;
        smCoords.x = smCoords.x / smCoords.w / 2.0f + 0.5f;
        smCoords.y = -smCoords.y / smCoords.w / 2.0f + 0.5f;

        float shadowFactor = 25;
        float smDepth = 0;

        if (smCoords.x >= 0 && smCoords.x <= 1) {
            if (smCoords.y >= 0 && smCoords.y <= 1) {

                for (int ox = -2; ox < 3; ox++) {
                    for (int oy = -2; oy < 3; oy++) {
                        float2 offset = float2(float(ox) / 2048.0f, float(oy) / 2048.0f);
                        smDepth = shadowMaps[i].SampleLevel(g_sampler_sm, smCoords.xy + offset, 0).r;

                        if (((smCoords.z / smCoords.w) - 0.001f) > smDepth) {
                            shadowFactor = shadowFactor - 1;
                        }
                    }
                }
                if (shadowFactor == 0) {
                    continue;
                }
            }
        }

        shadowFactor = shadowFactor / 25;

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
        float specularPower = pow(max(dot(viewDir, reflectDir), 0.0), 32) * INspecularity.x;

        //Direction
        float3 lightDirNorm = normalize(lightDirection[i].xyz);
        float directionPower = pow(max((dot(-lightPositionNorm.xyz, lightDirNorm.xyz)), 0.0), 4);

        OUT_Color.xyz = OUT_Color.xyz + (tempLightColor.xyz + INalbedo.xyz) * tempLightPower * (normalPower + specularPower) * directionPower * shadowFactor;
    }

    float3 ambientLightColor = ambientLight.xyz / 4;
    float ambientLightPower = distance(float3(0, 0, 0), ambientLight.xyz) * 0.25f;

    OUT_Color.xyz = OUT_Color.xyz + ((INalbedo.xyz + ambientLightColor) * ambientLightPower * (INspecularity.x / 10.0f + 1.0f));

    PS_OUT result;

    result.lightingOutput = OUT_Color * INssao;

    const float brightness = dot(OUT_Color.rgb, float3(0.2126, 0.7152, 0.0722));
    if (brightness > 0.5) {
        result.bloomMap = float4(result.lightingOutput.rgb * INspecularity.g, INspecularity.g);
    } else {
        result.bloomMap = float4(0, 0, 0, 0);
    }

    return result;
}
