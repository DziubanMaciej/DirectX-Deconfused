struct VertexShaderInput {
    float3 Position : POSITION;
};

struct VertexShaderOutput {
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexShaderInput IN) {
    VertexShaderOutput OUT;

    OUT.Position.x = IN.Position.x;
    OUT.Position.y = IN.Position.y;
    OUT.Position.z = 1;
    OUT.Position.w = 1;

    return OUT;
}
