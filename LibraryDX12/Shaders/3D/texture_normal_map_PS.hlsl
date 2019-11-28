struct ObjectProperties {
    float3 albedoColor;
    float specularity;
    float bloomFactor;
};

ConstantBuffer<ObjectProperties> op : register(b1);
Texture2D normalMap : register(t0);
Texture2D diffuseTexture : register(t1);
SamplerState linearSampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float3x3 tbn :TBN;
};

struct PixelShaderOutput {
    float4 gBufferAlbedo;
    float4 gBufferNormal;
    float2 gBufferSpecular;
};

PixelShaderOutput main(PixelShaderInput IN) : SV_Target {
    const float2 textureCoords = float2(IN.UV.x, 1 - IN.UV.y);

    const float3 normalInTangentSpace = (2 * normalMap.Sample(linearSampler, textureCoords).xyz ) - float3(1, 1, 1);
    const float3 normalInWorldSpace = normalize(mul(IN.tbn, normalInTangentSpace));

    PixelShaderOutput result;
    result.gBufferNormal.xyz = normalInWorldSpace;
    result.gBufferNormal.w = 1;
    result.gBufferAlbedo = diffuseTexture.Sample(linearSampler, textureCoords);
    result.gBufferSpecular = float2(op.specularity, op.bloomFactor);
    return result;
}
