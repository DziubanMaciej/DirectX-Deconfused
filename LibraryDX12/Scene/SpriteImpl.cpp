#include "SpriteImpl.h"

#include "Application/ApplicationImpl.h"
#include "Resource/TextureImpl.h"

namespace DXD {
std::unique_ptr<Sprite> Sprite::create(Texture &texture, int textureSizeX, int textureOffsetX, int textureSizeY, int textureOffsetY,
                                       HorizontalAlignment horizontalAlignment, VerticalAlignment verticalAlignment) {
    return std::unique_ptr<Sprite>(new SpriteImpl(*static_cast<TextureImpl *>(&texture), textureSizeX, textureOffsetX, textureSizeY, textureOffsetY, horizontalAlignment, verticalAlignment));
}
} // namespace DXD

SpriteImpl::SpriteImpl(TextureImpl &texture, int textureSizeX, int textureOffsetX, int textureSizeY, int textureOffsetY,
                       HorizontalAlignment horizontalAlignment, VerticalAlignment verticalAlignment)
    : texture(texture), textureSizeX(textureSizeX), textureOffsetX(textureOffsetX), textureSizeY(textureSizeY), textureOffsetY(textureOffsetY), horizontalAlignment(horizontalAlignment), verticalAlignment(verticalAlignment) {
}

SpriteCB SpriteImpl::getData() {
    return {
        0.f,
        0.f,
        static_cast<float>(textureSizeX),
        static_cast<float>(textureOffsetX),
        static_cast<float>(textureSizeY),
        static_cast<float>(textureOffsetY),
        static_cast<unsigned>(horizontalAlignment),
        static_cast<unsigned>(verticalAlignment)
    };
}
