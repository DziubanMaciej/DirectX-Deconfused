#include "MeshImpl.h"
#include <fstream>

namespace DXD {
std::unique_ptr<Mesh> Mesh::create() {
    return std::unique_ptr<Mesh>{new MeshImpl()};
}
} // namespace DXD

MeshImpl::MeshImpl() {
    meshType = MeshType::NONE;
}

MeshImpl::~MeshImpl() {
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
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            break;
        case 'f': //faces, TODO texture coord and normals
            inputFile >> i1;
            inputFile >> i2;
            inputFile >> i3;
            indices.push_back(i1 - 1);
            indices.push_back(i2 - 1);
            indices.push_back(i3 - 1);
            break;
        }
    }

    inputFile.close();

    meshType = MeshType::TRIANGLE_STRIP;

    return 0;
}
