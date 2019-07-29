cbuffer SimpleConstantBuffer : register(b1) {
    int lightsSize;
    float3 ambientLight;
    float4 lightPosition[8];
    float4 lightColor[8];
};

Texture2D DiffuseTexture : register(t0);

struct PixelShaderInput {
    float4 WorldPosition : COLOR;
    float4 Normal : NORMAL;
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    float4 OUT_Color = float4(0, 0, 0, 1);
    OUT_Color.xyz = OUT_Color.xyz + ambientLight.xyz;

    for (int i = 0; i < lightsSize; i++) {
        float3 tempLightColor = lightColor[i].xyz;
        float tempLightPower = (20 / (distance(IN.WorldPosition.xyz, lightPosition[i].xyz) * distance(IN.WorldPosition.xyz, lightPosition[i].xyz))) * lightColor[i].w;
        float3 lightPositionNorm = normalize(lightPosition[i].xyz - IN.WorldPosition.xyz);
        float3 normalNorm = normalize(IN.Normal.xyz);
        float normalPower = max(dot(normalNorm, lightPositionNorm), 0);
        OUT_Color.xyz = OUT_Color.xyz + tempLightColor.xyz * tempLightPower * normalPower;
        //OUT_Color.xyz = IN.Normal.xyz;
    }

    return OUT_Color;
}
