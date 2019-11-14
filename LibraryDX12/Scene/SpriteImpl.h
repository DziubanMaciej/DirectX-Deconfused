#pragma once

#include "ConstantBuffers/ConstantBuffers.h"
#include "Resource/ConstantBuffer.h"

#include "DXD/Sprite.h"
#include "DXD/Texture.h"

#include <ExternalHeaders/Wrappers/d3dx12.h>
class TextureImpl;
class SpriteImpl : public DXD::Sprite {
protected:
    friend class DXD::Sprite;

public:
    SpriteImpl(TextureImpl &texture, int textureSizeX, int textureOffsetX, int textureSizeY, int textureOffsetY,
               HorizontalAlignment horizontalAlignment, VerticalAlignment verticalAlignment);
    ~SpriteImpl() = default;

    TextureImpl &getTextureImpl() { return texture; }
    SpriteCB getData();

private:
    TextureImpl &texture;
    int textureSizeX;
    int textureOffsetX;
    int textureSizeY;
    int textureOffsetY;
    DXD::Sprite::HorizontalAlignment horizontalAlignment;
    DXD::Sprite::VerticalAlignment verticalAlignment;
};
