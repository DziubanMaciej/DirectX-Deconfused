struct ModelViewProjection {
    matrix mvp;
};

struct SimpleConstantBuffer {
    int lightsSize;
    float3 lightPosition;
    float3 lightColor;
};

ConstantBuffer<ModelViewProjection> mvp : register(b0);
ConstantBuffer<SimpleConstantBuffer> simpleConstantBuffer : register(b1);

struct VertexShaderInput {
    float3 Position : POSITION;
};

struct VertexShaderOutput {
    float4 Color : COLOR;
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexShaderInput IN) {
    VertexShaderOutput OUT;

    OUT.Position = mul(mvp.mvp, float4(IN.Position, 1.0f));
    OUT.Color = float4(0.f, 0.f, 0.f, 1);

    if (simpleConstantBuffer.lightsSize >= 1) {
        float3 lightColor = simpleConstantBuffer.lightColor * 0.01;
        float lightPower = 2000 / (distance(IN.Position.xyz, simpleConstantBuffer.lightPosition) * distance(IN.Position.xyz, simpleConstantBuffer.lightPosition));
        OUT.Color.xyz += lightColor.xyz * lightPower;
    }

    return OUT;
}
