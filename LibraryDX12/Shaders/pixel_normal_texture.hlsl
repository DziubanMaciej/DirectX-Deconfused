cbuffer SimpleConstantBuffer : register(b1) {
    float4 cameraPosition;
    int lightsSize;
    float3 ambientLight;
    float4 lightPosition[8];
    float4 lightColor[8];
    float4 lightDirection[8];
    matrix smVpMatrix[8];
};

struct ObjectProperties {
    float3 objectColor;
    float objectSpecularity;
};

ConstantBuffer<ObjectProperties> op : register(b2);

Texture2D shadowMap0 : register(t0);
Texture2D shadowMap1 : register(t1);
Texture2D shadowMap2 : register(t2);
Texture2D shadowMap3 : register(t3);
Texture2D shadowMap4 : register(t4);
Texture2D shadowMap5 : register(t5);
Texture2D shadowMap6 : register(t6);
Texture2D shadowMap7 : register(t7);

Texture2D diffuseTexture : register(t8);

SamplerState s_sampler : register(s0);

struct PixelShaderInput {
    float4 WorldPosition : COLOR;
    float4 Normal : NORMAL;
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
};

float4 main(PixelShaderInput IN) : SV_Target {
    float4 OUT_Color = float4(0, 0, 0, 1);
    OUT_Color.xyz = OUT_Color.xyz + ambientLight.xyz;

    float2 textureCoords = float2(IN.UV.x, 1 - IN.UV.y);
    float4 objectTextureColor = diffuseTexture.Sample(s_sampler, textureCoords);

    for (int i = 0; i < lightsSize; i++) {

        //Check for shadow
        float4 smCoords = (mul(smVpMatrix[i], IN.WorldPosition)).xyzw;
        smCoords.x = smCoords.x / smCoords.w / 2.0f + 0.5f;
        smCoords.y = -smCoords.y / smCoords.w / 2.0f + 0.5f;

        float shadowFactor = 9;
        float smDepth = 0;

        if (smCoords.x >= 0 && smCoords.x <= 1) {
            if (smCoords.y >= 0 && smCoords.y <= 1) {

                for (int ox = -1; ox < 2; ox++) {
                    for (int oy = -1; oy < 2; oy++) {

                        float2 offset = float2(float(ox) / 2048.0f, float(oy) / 2048.0f);

                        if (i == 0) {
                            smDepth = shadowMap0.Sample(s_sampler, smCoords.xy + offset).r;
                        } else if (i == 1) {
                            smDepth = shadowMap1.Sample(s_sampler, smCoords.xy + offset).r;
                        } else if (i == 2) {
                            smDepth = shadowMap2.Sample(s_sampler, smCoords.xy + offset).r;
                        } else if (i == 3) {
                            smDepth = shadowMap3.Sample(s_sampler, smCoords.xy + offset).r;
                        } else if (i == 4) {
                            smDepth = shadowMap4.Sample(s_sampler, smCoords.xy + offset).r;
                        } else if (i == 5) {
                            smDepth = shadowMap5.Sample(s_sampler, smCoords.xy + offset).r;
                        } else if (i == 6) {
                            smDepth = shadowMap6.Sample(s_sampler, smCoords.xy + offset).r;
                        } else if (i == 7) {
                            smDepth = shadowMap7.Sample(s_sampler, smCoords.xy + offset).r;
                        }

                        if (((smCoords.z / smCoords.w) - 0.0001f) > smDepth) {
                            shadowFactor = shadowFactor - 1;
                        }
                    }
                }
                if (shadowFactor == 0) {
                    continue;
                }
            }
        }

        shadowFactor = shadowFactor / 9;

        //Light color
        float3 tempLightColor = lightColor[i].xyz / 3;

        //Diffuse
        float tempLightPower = (30 / (distance(IN.WorldPosition.xyz, lightPosition[i].xyz) * distance(IN.WorldPosition.xyz, lightPosition[i].xyz))) * lightColor[i].w;

        //Normal
        float3 lightPositionNorm = normalize(lightPosition[i].xyz - IN.WorldPosition.xyz);
        float3 normalNorm = normalize(IN.Normal.xyz);
        float normalPower = max(dot(normalNorm, lightPositionNorm), 0);

        //Specular
        float3 viewDir = normalize(cameraPosition.xyz - IN.WorldPosition.xyz);
        float3 reflectDir = reflect(-lightPositionNorm, normalNorm);
        float specularPower = pow(max(dot(viewDir, reflectDir), 0.0), 32) * op.objectSpecularity;

        //Direction
        float3 lightDirNorm = normalize(lightDirection[i].xyz);
        float directionPower = pow(max((dot(-lightPositionNorm.xyz, lightDirNorm.xyz)), 0.0), 4);

        OUT_Color.xyz = OUT_Color.xyz + (tempLightColor.xyz + objectTextureColor) * tempLightPower * (normalPower + specularPower) * directionPower * shadowFactor;
    }

    return OUT_Color;
}
