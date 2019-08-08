
struct PixelShaderInput {
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    float4 OUT_Color = float4(0, 0, 0, 1);

    return OUT_Color;
}
