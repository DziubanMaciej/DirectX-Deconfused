struct SimpleConstantBuffer {
    int lightsSize;
    float3 lightPosition;
    float3 lightColor;
};

ConstantBuffer<SimpleConstantBuffer> simpleConstantBuffer : register(b1);

Texture2D DiffuseTexture : register(t0);

struct PixelShaderInput {
    float4 Color : COLOR;
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    float4 OUT_Color = float4(0, 0, 0, 1);

	if (simpleConstantBuffer.lightsSize >= 1) {
        float3 lightColor = simpleConstantBuffer.lightColor * 0.01;
        float lightPower = 2000 / (distance(IN.Color.xyz, simpleConstantBuffer.lightPosition) * distance(IN.Color.xyz, simpleConstantBuffer.lightPosition));
        OUT_Color.xyz += lightColor.xyz * lightPower;
    }
    //OUT_Color.xyz = simpleConstantBuffer.lightColor;

    return OUT_Color;
}
