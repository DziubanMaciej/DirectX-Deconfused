#pragma once

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
    MeshImpl();
    ~MeshImpl();

public:
    int loadFromObj(const std::string filePath);
    FLOAT *getVertices() { return vertices.data(); }
    INT *getIndices() { return indices.data(); }
    FLOAT *getNormals() { return normals.data(); }
    FLOAT *getTextureCoordinates() { return textureCoordinates.data(); }

protected:
    std::vector<FLOAT> vertices;
    std::vector<INT> indices;
    std::vector<FLOAT> normals;
    std::vector<FLOAT> textureCoordinates;
    MeshType meshType;
};
