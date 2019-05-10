#include "MeshImpl.h"
#include <fstream>
#include <vector>

namespace DXD {
std::unique_ptr<Mesh> Mesh::create() {
    return std::unique_ptr<Mesh>{new MeshImpl()};
}
} // namespace DXD

MeshImpl::MeshImpl() {
    vertices = nullptr;
    indices = nullptr;
    normals = nullptr;
    textureCoordinates = nullptr;
    meshType = MeshType::NONE;
}

MeshImpl::~MeshImpl() {
    if (vertices != nullptr) {
        delete vertices;
    }
    if (indices != nullptr) {
        delete indices;
    }
    if (normals != nullptr) {
        delete normals;
    }
    if (textureCoordinates != nullptr) {
        delete textureCoordinates;
    }
}

/**
	Loads verices and indices from .obj file. Temporary solution.
*/
int MeshImpl::loadFromObj(const std::string filePath) {

    std::fstream inputFile;
    inputFile.open(filePath, std::ios::in);

    if (!inputFile.good()) {
        return -1;
    }

    std::vector<FLOAT> verticesTemp;
    std::vector<INT> indicesTemp;
    //std::vector<FLOAT> normalsTemp;
    //std::vector<FLOAT> textureCoordinatesTemp;

    char lineType;
    FLOAT x, y, z, w;
    INT i1, i2, i3;

    while (inputFile >> lineType) { //TODO change reading method in order to parse vertices with 4 values, parse faces with normals etc.
        switch (lineType) {
        case '#': //comment
            break;
        case 'v': //vertices
            inputFile >> x;
            inputFile >> y;
            inputFile >> z;
            verticesTemp.push_back(x);
            verticesTemp.push_back(y);
            verticesTemp.push_back(z);
            break;
        case 'f': //faces, TODO texture coord and normals
            inputFile >> i1;
            inputFile >> i2;
            inputFile >> i3;
            indicesTemp.push_back(i1);
            indicesTemp.push_back(i2);
            indicesTemp.push_back(i3);
            break;
        }
    }

    inputFile.close();

    int verticesTempSize = verticesTemp.size();
    int indicesTempSize = indicesTemp.size();

    vertices = new FLOAT[verticesTempSize];
    indices = new INT[indicesTempSize];

    for (int i = 0; i < verticesTempSize; i++) {
        vertices[i] = verticesTemp[i];
    }

    for (int i = 0; i < indicesTempSize; i++) {
        indices[i] = indicesTemp[i] - 1; //changing indexing from 1 to 0 based
    }

    meshType = MeshType::TRIANGLE_STRIP;

    return 0;
}
