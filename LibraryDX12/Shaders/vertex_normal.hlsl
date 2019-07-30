struct ModelViewProjection {
    matrix modelMatrix;
    matrix mvpMatrix;
};

ConstantBuffer<ModelViewProjection> mmvp : register(b0);

struct VertexShaderInput {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
};

struct VertexShaderOutput {
    float4 WorldPosition : COLOR;
    float4 Normal : NORMAL;
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexShaderInput IN) {
    VertexShaderOutput OUT;

    OUT.Position = mul(mmvp.mvpMatrix, float4(IN.Position, 1.0f));
    OUT.WorldPosition = mul(mmvp.modelMatrix, float4(IN.Position, 1.0f));
    OUT.Normal = mul(mmvp.modelMatrix, float4(IN.Normal, 0.0f));

    return OUT;
}
