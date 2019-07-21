#pragma once

#include "Api/ApplicationImpl.h"

#include "DXD/Mesh.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"

#include "Wrappers/Resource.h"
#include <vector>

class MeshImpl : public DXD::Mesh {
protected:
    enum class MeshType {
        NONE,
        TRIANGLE_STRIP,
        TRIANGLE_STRIP_WITH_COORDS,
        TRIANGLE_STRIP_WITH_NORMALS,
        TRIANGLE_STRIP_WITH_COORDS_NORMALS
    };

    friend class DXD::Mesh;
    MeshImpl(DXD::Application &application, MeshType meshType,
             std::vector<FLOAT> &&vertices, std::vector<UINT> &&indices,
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

    auto& getVertexBuffer() { return vertexBuffer; }
    auto& getIndexBuffer() { return indexBuffer; }
    const D3D12_VERTEX_BUFFER_VIEW &getVertexBufferView() const { return vertexBufferView; }
    const D3D12_INDEX_BUFFER_VIEW &getIndexBufferView() const { return indexBufferView; }

protected:
    ApplicationImpl &application;
    MeshType meshType;
    std::vector<FLOAT> vertices;
    std::vector<UINT> indices;
    std::vector<FLOAT> normals;
    std::vector<FLOAT> textureCoordinates;

    std::unique_ptr<Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    std::unique_ptr<Resource> indexBuffer;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;

private:
    void uploadToGPU();
};
