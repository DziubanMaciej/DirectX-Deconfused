struct ObjectProperties {
    float3 albedoColor;
    float specularity;
    float bloomFactor;
};

ConstantBuffer<ObjectProperties> op : register(b2);

struct PixelShaderInput {
    float4 Normal : NORMAL;
    float4 Position : SV_Position;
};

struct PS_OUT {
    float4 gBufferAlbedo;
    float4 gBufferNormal;
    float4 gBufferSpecular;
};

PS_OUT main(PixelShaderInput IN) : SV_Target {

    PS_OUT result;

    result.gBufferAlbedo = float4(op.albedoColor, 1);
    result.gBufferNormal = normalize(IN.Normal);
    result.gBufferSpecular = float4(op.specularity, op.bloomFactor, 0, 1);

    return result;
}
