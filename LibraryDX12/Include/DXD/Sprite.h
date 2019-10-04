#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <memory>
#include <string>

namespace DXD {

class Texture;

class EXPORT Sprite : NonCopyableAndMovable {
public:
    enum class VerticalAlignment {
        TOP,
        CENTER,
        BOTTOM
    };
    enum class HorizontalAlignment {
        LEFT,
        CENTER,
        RIGHT
    };
    static std::unique_ptr<Sprite> create(Texture &texture, int textureSizeX, int textureOffsetX, int textureSizeY, int textureOffsetY,
                                                  VerticalAlignment verticalAlignment, HorizontalAlignment horizontalAlignment);
    virtual ~Sprite() = default;

protected:
    Sprite() = default;
};

} // namespace DXD
