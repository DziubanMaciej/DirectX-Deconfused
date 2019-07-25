struct ModelViewProjection {
    matrix mvp;
};

ConstantBuffer<ModelViewProjection> mvp : register(b0);

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
    OUT.Color = float4(IN.Position, 1.0f);

    return OUT;
}
