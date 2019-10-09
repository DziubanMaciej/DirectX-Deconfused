#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <memory>
#include <string>

namespace DXD {

class Application;

class EXPORT Mesh : NonCopyableAndMovable {
public:
    virtual ~Mesh() = default;
    static std::unique_ptr<Mesh> createFromObj(DXD::Application &application, const std::wstring &filePath,
                                               bool loadTextureCoordinates, bool computeTangents, bool asynchronousLoading);

protected:
    Mesh() = default;
};

} // namespace DXD
