#pragma once

#include "ApplicationImpl.h"
#include "DXD/Mesh.h"
#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <vector>

enum MeshType {
    NONE,
    TRIANGLE_STRIP,
    TRIANGLE_STRIP_WITH_COORDS,
    TRIANGLE_STRIP_WITH_NORMALS,
    TRIANGLE_STRIP_WITH_COORDS_NORMALS
};

class MeshImpl : public DXD::Mesh {
protected:
    friend class DXD::Mesh;
    MeshImpl(DXD::Application &application);
    ~MeshImpl();

public:
    int loadFromObj(const std::string filePath);

    const FLOAT *getVertices() const { return vertices.data(); }
    const UINT *getIndices() const { return indices.data(); }
    const FLOAT *getNormals() const { return normals.data(); }
    const FLOAT *getTextureCoordinates() const { return textureCoordinates.data(); }

    size_t getVerticesCount() const { return vertices.size(); }
    size_t getIndicesCount() const { return indices.size(); }
    size_t getNormalsCount() const { return normals.size(); }
    size_t getTextureCoordinatesCount() const { return textureCoordinates.size(); }

    ID3D12ResourcePtr getVertexBuffer() const { return vertexBuffer; }
    ID3D12ResourcePtr getIndexBuffer() const { return indexBuffer; }
    const D3D12_VERTEX_BUFFER_VIEW &getVertexBufferView() const { return vertexBufferView; }
    const D3D12_INDEX_BUFFER_VIEW &getIndexBufferView() const { return indexBufferView; }

protected:
    ApplicationImpl &application;
    ID3D12ResourcePtr vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    ID3D12ResourcePtr indexBuffer;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    std::vector<FLOAT> vertices;
    std::vector<UINT> indices;
    std::vector<FLOAT> normals;
    std::vector<FLOAT> textureCoordinates;
    MeshType meshType;

private:
    void uploadToGPU();
};
