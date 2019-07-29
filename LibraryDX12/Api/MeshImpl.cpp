#include "MeshImpl.h"

#include "Api/ApplicationImpl.h"
#include "Utility/ThrowIfFailed.h"
#include "Wrappers/CommandList.h"

#include <fstream>
#include <string>

namespace DXD {
std::unique_ptr<Mesh> Mesh::createFromObj(DXD::Application &application, const std::string &filePath) {
    std::fstream inputFile{filePath, std::ios::in};
    if (!inputFile.good()) {
        return nullptr;
    }

    std::vector<FLOAT> vertices;           //temp
    std::vector<UINT> indices;             //optional
    std::vector<std::string> faces;        //temp
    std::vector<FLOAT> normals;            //temp
    std::vector<FLOAT> textureCoordinates; //temp
    std::vector<FLOAT> outputVertices;

    std::string lineType;
    FLOAT x, y, z;
    UINT i1, i2, i3;
    std::string f1, f2, f3;

    while (inputFile >> lineType) { //TODO change reading method in order to parse vertices with 4 values, parse faces with normals etc.
        if (lineType == "#") {      //comment
            continue;
        } else if (lineType == "v") { //vertices
            inputFile >> x;
            inputFile >> y;
            inputFile >> z;
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        } else if (lineType == "f") { //faces
            inputFile >> f1;
            inputFile >> f2;
            inputFile >> f3;
            faces.push_back(f1);
            faces.push_back(f2);
            faces.push_back(f3);
        } else if (lineType == "vn") { //normal vector
            inputFile >> x;
            inputFile >> y;
            inputFile >> z;
            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);
        } else if (lineType == "vt") { //texture vector
            inputFile >> x;
            inputFile >> y;
            inputFile >> z;
            textureCoordinates.push_back(x);
            textureCoordinates.push_back(y);
            textureCoordinates.push_back(z);
        }
    }

    for (std::string face : faces) {
        UINT vertexIdx;
        UINT normalIdx;
        UINT textCoordIdx;
        size_t pos = 0;
        std::string f = face;
        for (int i = 0; i < 3; i++) {
            pos = f.find("/");
            std::string t = f.substr(0, pos);
            switch (i) {
            case 0:
                vertexIdx = std::stoi(t);
                break;
            case 1:
                if (t.length() > 0) {
                    textCoordIdx = std::stoi(t);
                }
                break;
            case 2:
                if (t.length() > 0) {
                    normalIdx = std::stoi(t);
                }
                break;
            }
            f.erase(0, pos + 1);
        }
        indices.push_back(vertexIdx - 1);
        outputVertices.push_back(vertices[3 * (vertexIdx - 1)]);
        outputVertices.push_back(vertices[3 * (vertexIdx - 1) + 1]);
        outputVertices.push_back(vertices[3 * (vertexIdx - 1) + 2]);
        if (normals.size() > 0) {
            outputVertices.push_back(normals[3 * (normalIdx - 1)]);
            outputVertices.push_back(normals[3 * (normalIdx - 1) + 1]);
            outputVertices.push_back(normals[3 * (normalIdx - 1) + 2]);
        }
        if (textureCoordinates.size() > 0) {
            outputVertices.push_back(textureCoordinates[3 * (textCoordIdx - 1)]);
            outputVertices.push_back(textureCoordinates[3 * (textCoordIdx - 1) + 1]);
            outputVertices.push_back(textureCoordinates[3 * (textCoordIdx - 1) + 2]);
        }
    }

    if (normals.size() > 0) {
        if (textureCoordinates.size() > 0) {
            return std::unique_ptr<Mesh>{new MeshImpl(application, MeshImpl::MeshType::TRIANGLE_STRIP_WITH_COORDS_NORMALS,
                                                      std::move(outputVertices), 9 * sizeof(FLOAT), std::move(indices),
                                                      std::move(normals), std::move(textureCoordinates))};
        } else {
            return std::unique_ptr<Mesh>{new MeshImpl(application, MeshImpl::MeshType::TRIANGLE_STRIP_WITH_NORMALS,
                                                      std::move(outputVertices), 6 * sizeof(FLOAT), std::move(indices),
                                                      std::move(normals), std::move(textureCoordinates))};
        }
    } else {
        if (textureCoordinates.size() > 0) {
            return std::unique_ptr<Mesh>{new MeshImpl(application, MeshImpl::MeshType::TRIANGLE_STRIP_WITH_COORDS,
                                                      std::move(outputVertices), 6 * sizeof(FLOAT), std::move(indices),
                                                      std::move(normals), std::move(textureCoordinates))};
        } else {
            return std::unique_ptr<Mesh>{new MeshImpl(application, MeshImpl::MeshType::TRIANGLE_STRIP,
                                                      std::move(vertices), 3 * sizeof(FLOAT), std::move(indices),
                                                      std::move(normals), std::move(textureCoordinates))};
        }
    }

    return nullptr;
}
} // namespace DXD

MeshImpl::MeshImpl(DXD::Application &application, MeshType meshType,
                   std::vector<FLOAT> &&vertices, UINT vertexSize, std::vector<UINT> &&indices,
                   std::vector<FLOAT> &&normals, std::vector<FLOAT> &&textureCoordinates)
    : application(*static_cast<ApplicationImpl *>(&application)),
      meshType(meshType),
      vertices(std::move(vertices)),
      vertexSize(vertexSize),
      indices(std::move(indices)),
      normals(std::move(normals)),
      textureCoordinates(std::move(textureCoordinates)) {
    uploadToGPU();
}

void MeshImpl::uploadToGPU() {
    // Context
    ID3D12DevicePtr device = application.getDevice();
    auto &commandQueue = application.getDirectCommandQueue();

    // Record command list for GPU upload
    CommandList commandList{commandQueue.getCommandAllocatorManager(), nullptr};
    const UINT verticesCount = static_cast<UINT>(vertices.size() / (vertexSize / sizeof(FLOAT)));
    //const UINT vertexSize = 3 * sizeof(FLOAT);
    this->vertexBuffer = std::make_unique<VertexBuffer>(device, commandList, vertices.data(), verticesCount, vertexSize);
    this->indexBuffer = std::make_unique<IndexBuffer>(device, commandList, indices.data(), static_cast<UINT>(indices.size()));
    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    std::vector<CommandList *> commandLists{&commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);
}
