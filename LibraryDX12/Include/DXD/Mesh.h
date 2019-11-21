#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

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
    /// Factory function for loading geometry from wavefront obj file. Internally handles
    /// getting the geometry to the GPU memory and all operations associated with setting
    /// it up.
    /// \param application execution context for the loader
    /// \param filePath relative or absolute path of the obj file
    /// \param loadTextureCoordinates when set to true, adds UVs to the model. Fails if the
    /// uvs are not available
    /// \param computeTangents when set to true, calculates tangent vector for vertices to
    /// enable normal mapping
    /// \param asynchronousLoading when set to true, handles object loading in a separate thread
    /// managed by the engine
    /// \return created mesh
    static std::unique_ptr<Mesh> createFromObj(DXD::Application &application, const std::wstring &filePath,
                                               bool loadTextureCoordinates, bool computeTangents, bool asynchronousLoading);
    virtual ~Mesh() = default;

protected:
    Mesh() = default;
};

} // namespace DXD
