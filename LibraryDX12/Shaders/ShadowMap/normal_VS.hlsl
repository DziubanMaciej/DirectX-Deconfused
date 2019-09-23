struct SMmvp {
    matrix mvpMatrix;
};

ConstantBuffer<SMmvp> mvp : register(b0);

struct VertexShaderInput {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
};

struct VertexShaderOutput {
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexShaderInput IN) {
    VertexShaderOutput OUT;

    OUT.Position = mul(mvp.mvpMatrix, float4(IN.Position, 1.0f));

    return OUT;
}
