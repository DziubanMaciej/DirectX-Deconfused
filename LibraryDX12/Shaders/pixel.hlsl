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
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    float4 OUT_Color = float4(0, 0, 0, 1);

	for (int i = 0; i < lightsSize; i++) {
        float3 tempLightColor = lightColor[i].xyz * 0.01;
        float tempLightPower = 2000 / (distance(IN.WorldPosition.xyz, lightPosition[i]) * distance(IN.WorldPosition.xyz, lightPosition[i].xyz));
        OUT_Color.xyz = OUT_Color.xyz + tempLightColor.xyz * tempLightPower;
	}

    return OUT_Color;
}
