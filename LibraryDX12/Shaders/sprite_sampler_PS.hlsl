struct SpriteCB {
    float textureSizeX;
    float textureOffsetX;
    float textureSizeY;
    float textureOffsetY;
};

ConstantBuffer<SpriteCB> cb : register(b0);
Texture2D sprite : register(t0);
SamplerState g_sampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    const float uBase = (IN.Position.x - cb.textureOffsetX) / cb.textureSizeX;
    const float vBase = (IN.Position.y - cb.textureOffsetY) / cb.textureSizeY;
    if (IN.Position.x - cb.textureOffsetX > cb.textureSizeX || IN.Position.x - cb.textureOffsetX < 0 || IN.Position.y - cb.textureOffsetY > cb.textureSizeY || IN.Position.y - cb.textureOffsetY < 0)
        discard;
    return sprite.Sample(g_sampler, float2(uBase, vBase));
}