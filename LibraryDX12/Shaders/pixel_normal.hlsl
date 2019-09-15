struct ObjectProperties {
    float3 objectColor;
    float objectSpecularity;
};

ConstantBuffer<ObjectProperties> op : register(b2);

struct PixelShaderInput {
    float4 WorldPosition : COLOR;
    float4 Normal : NORMAL;
    float4 Position : SV_Position;
};

struct PS_OUT {
    float4 gBufferPosition;
    float4 gBufferAlbedo;
    float4 gBufferNormal;
    float4 gBufferSpecular;
};

PS_OUT main(PixelShaderInput IN) : SV_Target {

	PS_OUT result;

	result.gBufferPosition = IN.WorldPosition;
    result.gBufferAlbedo = float4(op.objectColor.xyz, 1);
    result.gBufferNormal = normalize(IN.Normal);
    result.gBufferSpecular = float4(op.objectSpecularity, op.objectSpecularity, op.objectSpecularity, 1);

    return result;
}
