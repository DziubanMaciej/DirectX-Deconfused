struct PostProcessLinearColorCorrectionCB {
    float4x3 colorMatrix;
    float screenWidth;
    float screenHeight;
};
ConstantBuffer<PostProcessLinearColorCorrectionCB> cb : register(b0);
Texture2D scene : register(t0);
SamplerState sceneSampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    // Get base color
    const float uBase = IN.Position.x / cb.screenWidth;
    const float vBase = IN.Position.y / cb.screenHeight;
    const float4 IN_Color = scene.Sample(sceneSampler, float2(uBase, vBase));

    // Convert matrix to 3x3
    const float3x3 matrix3x3 = float3x3(
        cb.colorMatrix[0][0], cb.colorMatrix[1][0], cb.colorMatrix[2][0],
        cb.colorMatrix[0][1], cb.colorMatrix[1][1], cb.colorMatrix[2][1],
        cb.colorMatrix[0][2], cb.colorMatrix[1][2], cb.colorMatrix[2][2]);

    // Apply correction
    return float4(mul(matrix3x3, IN_Color.rgb), IN_Color.a);
}
