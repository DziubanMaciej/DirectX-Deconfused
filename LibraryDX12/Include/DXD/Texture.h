#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <memory>
#include <string>

namespace DXD {

class Application;

class EXPORT Texture : NonCopyableAndMovable {
public:
    static std::unique_ptr<Texture> createFromFile(Application &application, const std::wstring &filePath, bool asynchronousLoading);
    virtual ~Texture() = default;

protected:
    Texture() = default;
};

} // namespace DXD
