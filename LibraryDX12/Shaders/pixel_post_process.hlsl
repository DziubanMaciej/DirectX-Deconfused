struct PostProcessCB {
    float screenWidth;
    float screenHeight;
};

ConstantBuffer<PostProcessCB> ppcb : register(b0);

Texture2D scene : register(t0);

SamplerState g_sampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    float4 OUT_Color = float4(0, 0, 0, 1);

    float pos_x = IN.Position.x / ppcb.screenWidth;
    float pos_y = IN.Position.y / ppcb.screenHeight;

    OUT_Color.xyz = scene.Sample(g_sampler, float2(pos_x, pos_y)).rgb;

	if (pos_y < 0.05 || pos_y > 0.95) {
        return float4(0, 1, 0, 1);
	}

    return OUT_Color;
}
