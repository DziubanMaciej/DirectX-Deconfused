struct ObjectProperties {
    float3 albedoColor;
    float specularity;
    float bloomFactor;
};

ConstantBuffer<ObjectProperties> op : register(b1);
Texture2D normalMap : register(t0);
Texture2D diffuseTexture : register(t1);
SamplerState s_sampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
    float3 Tangent : TANGENT;
    float2 UV : TEXCOORD;
};

struct PixelShaderOutput {
    float4 gBufferAlbedo;
    float4 gBufferNormal;
    float2 gBufferSpecular;
};

PixelShaderOutput main(PixelShaderInput IN) : SV_Target {
    const float2 textureCoords = float2(IN.UV.x, 1 - IN.UV.y);

    PixelShaderOutput result;
    result.gBufferAlbedo = diffuseTexture.Sample(s_sampler, textureCoords);
    result.gBufferNormal = 2 * normalMap.Sample(s_sampler, textureCoords) - float4(1, 1, 1, 1);
    result.gBufferSpecular = float2(op.specularity, op.bloomFactor);
    return result;
}
