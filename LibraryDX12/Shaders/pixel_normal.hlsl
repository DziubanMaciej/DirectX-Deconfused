cbuffer SimpleConstantBuffer : register(b1) {
    int lightsSize;
    int pad;
    int padd;
    int paddd;
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

    for (int i = 0; i < lightsSize; i++) {
        float3 tempLightColor = lightColor[i].xyz;
        float tempLightPower = 20 / (distance(IN.WorldPosition.xyz, lightPosition[i].xyz) * distance(IN.WorldPosition.xyz, lightPosition[i].xyz));
        float3 lightPositionNorm = normalize(IN.WorldPosition.xyz - lightPosition[i].xyz);
        float3 normalNorm = IN.Normal.xyz;
        float normalPower = max(dot(normalNorm, lightPositionNorm), 0);
        OUT_Color.xyz = OUT_Color.xyz + tempLightColor.xyz * tempLightPower * normalPower;
        //OUT_Color.xyz = IN.Normal.xyz;
    }

    return OUT_Color;
}
