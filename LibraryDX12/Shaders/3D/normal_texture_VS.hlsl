struct NormalTextureCB {
    matrix modelMatrix;
    matrix mvpMatrix;
    float2 textureScale;
};

ConstantBuffer<NormalTextureCB> cb : register(b0);

struct VertexShaderInput {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct VertexShaderOutput {
    float4 Normal : NORMAL;
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
};

VertexShaderOutput main(VertexShaderInput IN) {
    VertexShaderOutput OUT;
    OUT.Position = mul(cb.mvpMatrix, float4(IN.Position, 1.0f));
    OUT.Normal = mul(cb.modelMatrix, float4(IN.Normal, 0.0f));
    OUT.UV = IN.UV * cb.textureScale;
    return OUT;
}
