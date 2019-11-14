struct ShadowMapCB {
    matrix mvp;
};

ConstantBuffer<ShadowMapCB> cb : register(b0);

struct VertexShaderInput {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct VertexShaderOutput {
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexShaderInput IN) {
    VertexShaderOutput OUT;

    OUT.Position = mul(cb.mvp, float4(IN.Position, 1.0f));

    return OUT;
}
