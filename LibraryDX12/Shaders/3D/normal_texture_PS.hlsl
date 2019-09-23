struct ObjectProperties {
    float3 albedoColor;
    float specularity;
    float bloomFactor;
};

ConstantBuffer<ObjectProperties> op : register(b2);

Texture2D diffuseTexture : register(t0);

SamplerState s_sampler : register(s0);

struct PixelShaderInput {
    float4 Normal : NORMAL;
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
};

struct PS_OUT {
    float4 gBufferAlbedo;
    float4 gBufferNormal;
    float4 gBufferSpecular;
};

PS_OUT main(PixelShaderInput IN) : SV_Target {

    float2 textureCoords = float2(IN.UV.x, 1 - IN.UV.y);
    float4 objectTextureColor = diffuseTexture.Sample(s_sampler, textureCoords);

    PS_OUT result;

    result.gBufferAlbedo = objectTextureColor;
    result.gBufferNormal = normalize(IN.Normal);
    result.gBufferSpecular = float4(op.specularity, op.bloomFactor, 0, 1);

    return result;
}
