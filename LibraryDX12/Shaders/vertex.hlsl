struct ModelViewProjection {
    matrix modelMatrix;
    matrix mvpMatrix;
};

ConstantBuffer<ModelViewProjection> mmvp : register(b0);

struct VertexShaderInput {
    float3 Position : POSITION;
};

struct VertexShaderOutput {
    float4 Color : COLOR;
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexShaderInput IN) {
    VertexShaderOutput OUT;

    OUT.Position = mul(mmvp.mvpMatrix, float4(IN.Position, 1.0f));
    OUT.Color = mul(mmvp.modelMatrix, float4(IN.Position, 1.0f));

    return OUT;
}
