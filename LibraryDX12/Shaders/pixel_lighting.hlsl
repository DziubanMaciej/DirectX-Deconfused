cbuffer SimpleConstantBuffer : register(b1) {
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


Texture2D gBufferPosition : register(t0);
Texture2D gBufferAlbedo : register(t1);
Texture2D gBufferNormal : register(t2);
Texture2D gBufferSpecular : register(t3);
Texture2D shadowMaps[8] : register(t4);

SamplerState g_sampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

struct PS_OUT {
    float4 scene;
    float4 bloomMap;
};

PS_OUT main(PixelShaderInput IN) : SV_Target {
    float4 OUT_Color = float4(0, 0, 0, 1);

	const float uBase = IN.Position.x / screenWidth;
    const float vBase = IN.Position.y / screenHeight;

	float4 INworldPosition = gBufferPosition.Sample(g_sampler, float2(uBase, vBase));
    float4 INnormal = gBufferNormal.Sample(g_sampler, float2(uBase, vBase));
    float4 INspecularity = gBufferSpecular.Sample(g_sampler, float2(uBase, vBase));
    float4 INalbedo = gBufferAlbedo.Sample(g_sampler, float2(uBase, vBase));

    OUT_Color.xyz = OUT_Color.xyz + ambientLight.xyz;

    for (int i = 0; i < lightsSize; i++) {

        //Check for shadow
        float4 smCoords = (mul(smVpMatrix[i], INworldPosition)).xyzw;
        smCoords.x = smCoords.x / smCoords.w / 2.0f + 0.5f;
        smCoords.y = -smCoords.y / smCoords.w / 2.0f + 0.5f;

        float shadowFactor = 9;
        float smDepth = 0;

        if (smCoords.x >= 0 && smCoords.x <= 1) {
            if (smCoords.y >= 0 && smCoords.y <= 1) {

                for (int ox = -1; ox < 2; ox++) {
                    for (int oy = -1; oy < 2; oy++) {
                        float2 offset = float2(float(ox) / 2048.0f, float(oy) / 2048.0f);
                        smDepth = shadowMaps[i].Sample(g_sampler, smCoords.xy + offset).r;

                        if (((smCoords.z / smCoords.w) - 0.0005f) > smDepth) {
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
        float tempLightPower = (30 / (distance(INworldPosition.xyz, lightPosition[i].xyz) * distance(INworldPosition.xyz, lightPosition[i].xyz))) * lightColor[i].w;

        //Normal
        float3 lightPositionNorm = normalize(lightPosition[i].xyz - INworldPosition.xyz);
        float3 normalNorm = normalize(INnormal.xyz);
        float normalPower = max(dot(normalNorm, lightPositionNorm), 0);

        //Specular
        float3 viewDir = normalize(cameraPosition.xyz - INworldPosition.xyz);
        float3 reflectDir = reflect(-lightPositionNorm, normalNorm);
        float specularPower = pow(max(dot(viewDir, reflectDir), 0.0), 32) * INspecularity.x;

        //Direction
        float3 lightDirNorm = normalize(lightDirection[i].xyz);
        float directionPower = pow(max((dot(-lightPositionNorm.xyz, lightDirNorm.xyz)), 0.0), 4);

        OUT_Color.xyz = OUT_Color.xyz + (tempLightColor.xyz + INalbedo) * tempLightPower * (normalPower + specularPower) * directionPower * shadowFactor;
    }

	PS_OUT result;

	result.scene = OUT_Color;

    //float brightness = dot(OUT_Color.rgb, float3(0.2126, 0.7152, 0.0722));
    //if (brightness > 0.5) {
    //    result.bloomMap = result.scene;
    //} else {
        result.bloomMap = float4(0, 0, 0, 1); // TO-DO bloom
    //}

    return result;
}
