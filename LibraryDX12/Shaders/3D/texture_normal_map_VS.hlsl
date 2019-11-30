struct TextureNormalMapCB {
    matrix modelMatrix;
    matrix modelViewProjectionMatrix;
    float2 textureScale;
    float _padding;
    uint normalMapAvailable;
};

ConstantBuffer<TextureNormalMapCB> cb : register(b0);

struct VertexShaderInput {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 UV : TEXCOORD;
};

struct VertexShaderOutput {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float3x3 tbn : TBN;
};

VertexShaderOutput main(VertexShaderInput IN) {
    VertexShaderOutput OUT;
    OUT.Position = mul(cb.modelViewProjectionMatrix, float4(IN.Position, 1.0f));
    OUT.UV = IN.UV * cb.textureScale;

    if (cb.normalMapAvailable) {
        // Pixel shader will sample normal from the normal map and use TBN matrix to transform it
        const float3 bitangent = cross(IN.Normal, IN.Tangent);
        OUT.tbn = transpose(float3x3(IN.Tangent, bitangent, IN.Normal));
        OUT.tbn = mul(cb.modelMatrix, OUT.tbn);
    } else {
        // Pixel shader will use per-vertex normal, we can pass it in the matrix
        const float3 normal = mul(cb.modelMatrix, IN.Normal);
        OUT.tbn = float3x3(normal, float3(0, 0, 0), float3(0, 0, 0));
    }

    return OUT;
}
