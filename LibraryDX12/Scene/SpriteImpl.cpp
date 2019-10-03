#include "SpriteImpl.h"

#include "Application/ApplicationImpl.h"
#include "Resource/TextureImpl.h"

namespace DXD {
std::unique_ptr<Sprite> Sprite::createFromFile(Application &application, const std::wstring &filePath, int textureSizeX, int textureOffsetX, int textureSizeY, int textureOffsetY,
                                               VerticalAlignment verticalAlignment, HorizontalAlignment horizontalAlignment) {
    return std::unique_ptr<Sprite>(new SpriteImpl(*static_cast<ApplicationImpl *>(&application), filePath, textureSizeX, textureOffsetX, textureSizeY, textureOffsetY, verticalAlignment, horizontalAlignment));
}
} // namespace DXD

SpriteImpl::SpriteImpl(ApplicationImpl &application, const std::wstring filePath, int textureSizeX, int textureOffsetX, int textureSizeY, int textureOffsetY,
                       VerticalAlignment verticalAlignment, HorizontalAlignment horizontalAlignment)
    : textureSizeX(textureSizeX), textureOffsetX(textureOffsetX), textureSizeY(textureSizeY), textureOffsetY(textureOffsetY), verticalAlignment(verticalAlignment), horizontalAlignment(horizontalAlignment) {
    texture = DXD::Texture::createFromFile(application, filePath, false);
}

TextureImpl *SpriteImpl::getTextureImpl() {
    return static_cast<TextureImpl *>(texture.get());
}

SpriteCB SpriteImpl::getData() {
    return {
        0.f,
        0.f,
        static_cast<float>(textureSizeX),
        static_cast<float>(textureOffsetX),
        static_cast<float>(textureSizeY),
        static_cast<float>(textureOffsetY),
        static_cast<unsigned>(verticalAlignment),
        static_cast<unsigned>(horizontalAlignment),

    };
}
