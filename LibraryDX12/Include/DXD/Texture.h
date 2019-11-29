#pragma once

#include "DXD/Event.h"
#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <memory>
#include <string>

namespace DXD {

class Application;

/// \brief 2D Texture stored on GPU
class EXPORT Texture : NonCopyableAndMovable {
public:
    /// Expected usage of the texture. This gives engine knowledge about how it should treat the texture, e.g.
    /// normal maps are forced to linear formats (no SRGB allowed).
    enum class TextureType {
        ALBEDO,
        NORMAL_MAP,
    };

    enum class TextureLoadResult {
        SUCCESS,
        WRONG_FILENAME,
        WRONG_TEXTURE,
    };
    using TextureLoadEvent = Event<TextureLoadResult>;

    /// Factory method handling reading and decoding texture from file store on disk, uploading it on GPU and
    /// performing all the necessary setup. Its instances can be reused in different contexts, e.g multiple
    /// Object instances. The operation is done synchronously - in the calling thread
    /// \param filePath relative or absolute path of the obj file
    /// \param type expected usage of texture
    /// \param loadResult optional parameter for checking operation status. Application should use it
    /// to verify if the loading succeeded.
    /// \return created textue
    static std::unique_ptr<Texture> loadFromFileSynchronously(const std::wstring &filePath, TextureType type, TextureLoadResult *loadResult);

    /// Factory method handling reading and decoding texture from file store on disk, uploading it on GPU and
    /// performing all the necessary setup. Its instances can be reused in different contexts, e.g multiple
    /// Object instances. The operation is done asynchronously, in background thread managed by the engine.
    /// \param filePath relative or absolute path of the obj file
    /// \param type expected usage of texture
    /// \param loadResult optional parameter for checking operation status. Application should use it
    /// to verify if the loading succeeded.
    /// \return created textue
    static std::unique_ptr<Texture> loadFromFileAsynchronously(const std::wstring &filePath, TextureType type, TextureLoadEvent *loadEvent);
    virtual ~Texture() = default;

protected:
    Texture() = default;
};

} // namespace DXD
