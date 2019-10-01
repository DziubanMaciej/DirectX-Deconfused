struct NormalTextureCB {
    matrix modelMatrix;
    matrix mvpMatrix;
    float2 textureScale;
};

ConstantBuffer<NormalTextureCB> cb : register(b0);

struct VertexShaderInput {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 UV : TEXCOORD;
};

struct VertexShaderOutput {
    float4 Position : SV_Position;
    float3 Tangent : TANGENT;
    float2 UV : TEXCOORD;
};

VertexShaderOutput main(VertexShaderInput IN) {
    VertexShaderOutput OUT;
    OUT.Position = mul(cb.mvpMatrix, float4(IN.Position, 1.0f));
    OUT.Tangent = IN.Tangent;
    OUT.UV = IN.UV * cb.textureScale;
    return OUT;
}
