#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"

#include <memory>
#include <string>

namespace DXD {

class Application;

class EXPORT Texture : NonCopyableAndMovable {
public:
    static std::unique_ptr<Texture> createFromFile(Application &application, const std::wstring &filePath);
    virtual ~Texture() = default;

protected:
    Texture() = default;
};

} // namespace DXD
