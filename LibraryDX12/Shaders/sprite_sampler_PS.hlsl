struct SpriteCB {
    float screenWidth;
    float screenHeight;
    float textureSizeX;
    float textureOffsetX;
    float textureSizeY;
    float textureOffsetY;
    uint verticalAlignment;
    uint horizontalAlignment;
};

ConstantBuffer<SpriteCB> cb : register(b0);
Texture2D sprite : register(t0);
SamplerState g_sampler : register(s0);

struct PixelShaderInput {
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    const float vBase = cb.verticalAlignment == 0 ? IN.Position.x - cb.textureOffsetX                                                                         // left case
                                                  : cb.verticalAlignment == 2 ? cb.screenWidth - IN.Position.x - cb.textureOffsetX                            // right case
                                                                              : cb.screenWidth / 2 - IN.Position.x + cb.textureSizeX / 2 + cb.textureOffsetX; // center case

    const float hBase = cb.horizontalAlignment == 0 ? IN.Position.y - cb.textureOffsetY                                                                            // top case
                                                    : cb.horizontalAlignment == 2 ? IN.Position.y + cb.textureOffsetY - cb.screenHeight + cb.textureSizeY          // bottom case
                                                                                  : IN.Position.y - cb.textureOffsetY - cb.screenHeight / 2 + cb.textureSizeY / 2; // center case
    if (vBase >= cb.textureSizeX || vBase <= 0 || hBase >= cb.textureSizeY || hBase <= 0)
        discard;
    return sprite.Sample(g_sampler, float2(vBase / cb.textureSizeX, hBase / cb.textureSizeY));
}