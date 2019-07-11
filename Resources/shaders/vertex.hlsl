struct ModelViewProjection {
    matrix MVP;
    int lightsSize;
    float3 lightPosition;
    float3 lightColor;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

struct VertexShaderInput {
    float3 Position : POSITION;
};

struct VertexShaderOutput {
    float4 Color : COLOR;
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexShaderInput IN) {
    VertexShaderOutput OUT;

    OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
    OUT.Color = float4(0.f, 0.f, 0.f, 1);

    if (ModelViewProjectionCB.lightsSize >= 1) {
        float3 lightColor = ModelViewProjectionCB.lightColor * 0.01;
        float lightPower = (distance(IN.Position.xyz, ModelViewProjectionCB.lightPosition) * distance(IN.Position.xyz, ModelViewProjectionCB.lightPosition)) / 5;
        OUT.Color.xyz += lightColor.xyz * lightPower;
    }

    return OUT;
}
