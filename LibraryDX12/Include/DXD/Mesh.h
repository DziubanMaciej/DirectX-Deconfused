#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/Event.h>
#include <memory>
#include <string>

namespace DXD {

class Application;

/// \brief Geometry of 3D object
///
/// Represents a mesh of vertices, possibly connected using indices that can be set to
/// DXD::Object instances. Class is able to store various vertex attributes inside the
/// vertices such as normals, tangents or texture coordinates. The engine internally
/// ignores objects with meshes that have not been loaded properly or are still loading.
class EXPORT Mesh : NonCopyableAndMovable {
public:
    enum class ObjLoadResult {
        SUCCESS,
        TERMINATED,
        WRONG_FILENAME,
        WRONG_OBJ,
    };
    using ObjLoadEvent = Event<ObjLoadResult>;

    /// Factory function for loading geometry from wavefront obj file synchronously, in the calling
    /// thread. Internally handles getting the geometry to the GPU memory and all operations
    /// associated with setting it up.
    /// \param filePath relative or absolute path of the obj file
    /// \param loadTextureCoordinates when set to true, adds UVs to the model. Fails if the uvs are
    /// not available
    /// \param computeTangents when set to true, calculates tangent vector for vertices to
    /// enable normal mapping
    /// \param loadResult optional parameter for checking operation status. Application should use it
    /// to verify if the loading succeeded.
    /// \return created mesh
    static std::unique_ptr<Mesh> createFromObjSynchronously(const std::wstring &filePath, bool loadTextureCoordinates,
                                                            bool computeTangents, ObjLoadResult *loadResult);

    /// Factory function for loading geometry from wavefront obj file asynchronously, in a background
    /// thread managed by the engine. Internally handles getting the geometry to the GPU memory and
    ///all operations associated with setting it up.
    /// \param filePath relative or absolute path of the obj file
    /// \param loadTextureCoordinates when set to true, adds UVs to the model. Fails if the uvs are
    /// not available
    /// \param computeTangents when set to true, calculates tangent vector for vertices to
    /// enable normal mapping
    /// \param loadEvent optional parameter for checking operation status. Application should use it
    /// to verify if the loading succeeded.
    /// \return created mesh
    static std::unique_ptr<Mesh> createFromObjAsynchronously(const std::wstring &filePath, bool loadTextureCoordinates,
                                                             bool computeTangents, ObjLoadEvent *loadEvent);
    virtual ~Mesh() = default;

protected:
    Mesh() = default;
};

} // namespace DXD
