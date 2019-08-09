#pragma once

#include "Application/ApplicationImpl.h"
#include "Resource/Resource.h"
#include "Resource/VertexOrIndexBuffer.h"

#include "DXD/Mesh.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <utility>
#include <vector>

class MeshImpl : public DXD::Mesh {
public:
    using MeshType = unsigned int;
    constexpr static MeshType UNKNOWN = 0x0;
    constexpr static MeshType TRIANGLE_STRIP = 0x01;
    constexpr static MeshType TEXTURE_COORDS = 0x02;
    constexpr static MeshType NORMALS = 0x04;

protected:
    friend class DXD::Mesh;
    MeshImpl(DXD::Application &application, MeshType meshType, const std::vector<FLOAT> &vertices,
             UINT vertexSize, const std::vector<UINT> &indices);
    ~MeshImpl() = default;

public:
    UINT getVertexSizeInBytes() const { return vertexSizeInBytes; }
    UINT getVerticesCount() const { return verticesCount; }
    UINT getIndicesCount() const { return indicesCount; }

    MeshType getMeshType() const { return meshType; }
    bool isUploadInProgress();

    auto &getVertexBuffer() { return vertexBuffer; }
    auto &getIndexBuffer() { return indexBuffer; }

protected:
    // Context and metadata
    ApplicationImpl &application;
    MeshType meshType;

    // CPU resources
    const UINT vertexSizeInBytes;
    const UINT verticesCount;
    const UINT indicesCount;

    // GPU resources
    std::unique_ptr<VertexBuffer> vertexBuffer;
    std::unique_ptr<IndexBuffer> indexBuffer;

private:
    static MeshType computeMeshType(const std::vector<FLOAT> &normals, const std::vector<FLOAT> &textureCoordinates, bool useTextures);
    static UINT computeVertexSize(MeshType meshType);
    void uploadToGPU(const std::vector<FLOAT> &vertices, const std::vector<UINT> &indices);
};
