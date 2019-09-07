cbuffer SimpleConstantBuffer : register(b1) {
    float4 cameraPosition;
    int lightsSize;
    float3 ambientLight;
    float4 lightPosition[8];
    float4 lightColor[8];
    float4 lightDirection[8];
};

struct ObjectProperties {
    float3 objectColor;
    float objectSpecularity;
};

ConstantBuffer<ObjectProperties> op : register(b2);

Texture2D DiffuseTexture : register(t0);

struct PixelShaderInput {
    float4 WorldPosition : COLOR;
    float4 Position : SV_Position;
};

struct PS_OUT {
    float4 scene;
    float4 bloomMap;
};

PS_OUT main(PixelShaderInput IN) : SV_Target {
    float4 OUT_Color = float4(0, 0, 0, 1);
    OUT_Color.xyz = OUT_Color.xyz + ambientLight.xyz;

    for (int i = 0; i < lightsSize; i++) {
        float3 tempLightColor = lightColor[i].xyz * 0.01;
        float tempLightPower = (2000 / (distance(IN.WorldPosition.xyz, lightPosition[i]) * distance(IN.WorldPosition.xyz, lightPosition[i].xyz))) * lightColor[1].w;

        //Direction
        float3 lightPositionNorm = normalize(lightPosition[i].xyz - IN.WorldPosition.xyz);
        float3 lightDirNorm = normalize(lightDirection[i].xyz);
        float directionPower = max(dot(-lightPositionNorm.xyz, lightDirNorm.xyz), 0.0);

        OUT_Color.xyz = OUT_Color.xyz + (tempLightColor.xyz + op.objectColor.xyz) * tempLightPower * directionPower;
    }

    PS_OUT result;
    result.scene = OUT_Color;

    float brightness = dot(OUT_Color.rgb, float3(0.2126, 0.7152, 0.0722));
    if (brightness > 0.5) {
        result.bloomMap = result.scene;
    } else {
        result.bloomMap = float4(0, 0, 0, 1);
    }

    return result;
}
