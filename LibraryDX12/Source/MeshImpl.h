#pragma once

#include "DXD/Mesh.h"
#include "DXD/ExternalHeadersWrappers/d3d12.h"

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
    FLOAT *getVertices() { return vertices; }
    INT *getIndices() { return indices; }
    FLOAT *getNormals() { return normals; }
    FLOAT *getTextureCoordinates() { return textureCoordinates; }

protected:
    FLOAT *vertices;
    INT *indices;
    FLOAT *normals;
    FLOAT *textureCoordinates;
    MeshType meshType;
};
