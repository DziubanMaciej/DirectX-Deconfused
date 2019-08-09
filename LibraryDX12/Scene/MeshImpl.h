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
    MeshImpl(DXD::Application &application, MeshType meshType,
             std::vector<FLOAT> &&vertices, UINT vertexSize, std::vector<UINT> &&indices,
             std::vector<FLOAT> &&normals, std::vector<FLOAT> &&textureCoordinates);
    ~MeshImpl() = default;

public:
    const FLOAT *getVertices() const { return vertices.data(); }
    const UINT *getIndices() const { return indices.data(); }
    const FLOAT *getNormals() const { return normals.data(); }
    const FLOAT *getTextureCoordinates() const { return textureCoordinates.data(); }

    size_t getVerticesCount() const { return vertices.size(); }
    size_t getIndicesCount() const { return indices.size(); }
    size_t getNormalsCount() const { return normals.size(); }
    size_t getTextureCoordinatesCount() const { return textureCoordinates.size(); }

    MeshType getMeshType() const { return meshType; }

    auto &getVertexBuffer() { return vertexBuffer; }
    auto &getIndexBuffer() { return indexBuffer; }

protected:
    // Context and metadata
    ApplicationImpl &application;
    MeshType meshType;
    UINT vertexSize;

    // CPU resources
    std::vector<FLOAT> vertices;
    std::vector<UINT> indices;
    std::vector<FLOAT> normals;
    std::vector<FLOAT> textureCoordinates;

    // GPU resources
    std::unique_ptr<VertexBuffer> vertexBuffer;
    std::unique_ptr<IndexBuffer> indexBuffer;

private:
    static MeshType computeMeshType(const std::vector<FLOAT> &normals, const std::vector<FLOAT> &textureCoordinates, bool useTextures);
    static UINT computeVertexSize(MeshType meshType);
    void uploadToGPU();
};
