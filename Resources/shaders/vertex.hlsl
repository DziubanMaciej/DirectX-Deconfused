struct ModelViewProjection {
    matrix MVP;
};
struct Model{
    matrix Model;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);
ConstantBuffer<Model> ModelCB : register(b1);

struct VertexShaderInput {
    float3 Position : POSITION;
    float2 TextureCoord : TEXCOORD2;
    float3 Normal : NORMAL;
};

struct VertexShaderOutput {
    float3 Normal : NORMAL;
    float3 WorldPosition : POSITION;
    float2 TextureCoord : TEXCOORD2;
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexShaderInput IN) {
    VertexShaderOutput OUT;

    OUT.Normal = mul(ModelCB.Model, float4(IN.Normal, 0.0f));
    OUT.WorldPosition = mul(ModelCB.Model, float4(IN.Position, 1.0f)).xyz;
    OUT.TextureCoord = IN.TextureCoord;
    OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));

    return OUT;
}
