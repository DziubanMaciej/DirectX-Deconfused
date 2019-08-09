#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"
#include <memory>
#include <string>

namespace DXD {

class Application;

class EXPORT Mesh : NonCopyableAndMovable {
public:
    virtual ~Mesh() = default;
    static std::unique_ptr<Mesh> createFromObj(DXD::Application &application, const std::string &filePath, bool useTextures);

protected:
    Mesh() = default;
};

} // namespace DXD
