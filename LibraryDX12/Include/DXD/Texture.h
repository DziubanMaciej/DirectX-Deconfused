#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <memory>
#include <string>

namespace DXD {

class Application;

/// \brief 2D Texture store on GPU
class EXPORT Texture : NonCopyableAndMovable {
public:
    /// Factory method handling reading and decoding texture from file store on disk, uploading it on GPU and
    /// performing all the necessary setup. Its instances can be reused in different contexts, e.g multiple
    /// Object instances.
    /// \param application execution context for the loader
    /// \param filePath relative or absolute path of the obj file
    /// \param asynchronousLoading when set to true, handles object loading in a separate thread
    /// managed by the engine
    /// \return created textue
    static std::unique_ptr<Texture> createFromFile(Application &application, const std::wstring &filePath, bool asynchronousLoading);
    virtual ~Texture() = default;

protected:
    Texture() = default;
};

} // namespace DXD
