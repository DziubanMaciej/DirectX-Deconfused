#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"

#include <memory>
#include <string>

namespace DXD {

class Application;

class EXPORT Sprite : NonCopyableAndMovable {
public:
    static std::unique_ptr<Sprite> createFromFile(Application &application, const std::wstring &filePath, int textureSizeX, int textureOffsetX, int textureSizeY, int textureOffsetY);
    virtual ~Sprite() = default;

protected:
    Sprite() = default;
};

} // namespace DXD
