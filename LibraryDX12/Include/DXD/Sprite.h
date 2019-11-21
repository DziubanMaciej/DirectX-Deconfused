#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <memory>
#include <string>

namespace DXD {

class Texture;

/// \brief 2D image presented on screen
class EXPORT Sprite : NonCopyableAndMovable {
public:
    enum class HorizontalAlignment {
        LEFT,
        CENTER,
        RIGHT
    };
    enum class VerticalAlignment {
        TOP,
        CENTER,
        BOTTOM
    };

    /// Factory method used to create Sprite instances
    /// \param texture image to display
    /// \param textureSizeX on- screen width of image, which will be scaled
    /// \param textureOffsetX on-screen horizontal offset of image
    /// \param textureSizeY on-screen height of image, which will be scaled
    /// \param textureOffsetY on-screen vertical offset of image
    /// \param horizontalAlignment on-screen horizontal alignment
    /// \param verticalAlignmeny on-screen vertical alignment
    /// \return Sprite instance created
    static std::unique_ptr<Sprite> create(Texture &texture, int textureSizeX, int textureOffsetX, int textureSizeY, int textureOffsetY,
                                          HorizontalAlignment horizontalAlignment, VerticalAlignment verticalAlignment);
    virtual ~Sprite() = default;

protected:
    Sprite() = default;
};

} // namespace DXD
