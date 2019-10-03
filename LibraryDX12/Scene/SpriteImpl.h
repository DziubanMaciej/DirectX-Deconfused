#pragma once

#include "Resource/ConstantBuffer.h"
#include "Resource/ConstantBuffers.h"

#include "DXD/Sprite.h"
#include "DXD/Texture.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"
class ApplicationImpl;
class TextureImpl;
class SpriteImpl : public DXD::Sprite {
protected:
    friend class DXD::Sprite;

public:
    SpriteImpl(ApplicationImpl &application, const std::wstring filePath, int textureSizeX, int textureOffsetX, int textureSizeY, int textureOffsetY);
    ~SpriteImpl() = default;

    TextureImpl *getTextureImpl();
    SpriteCB getData();

private:
    std::unique_ptr<DXD::Texture> texture;
    int textureSizeX;
    int textureOffsetX;
    int textureSizeY;
    int textureOffsetY;
};
