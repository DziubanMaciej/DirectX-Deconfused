#include "MeshImpl.h"

#include "Api/ApplicationImpl.h"
#include "Utility/ThrowIfFailed.h"
#include "Wrappers/CommandList.h"

#include <fstream>

namespace DXD {
std::unique_ptr<Mesh> Mesh::createFromObj(DXD::Application &application, const std::string &filePath) {
    std::fstream inputFile{filePath, std::ios::in};
    if (!inputFile.good()) {
        return nullptr;
    }

    std::vector<FLOAT> vertices;
    std::vector<UINT> indices;
    std::vector<FLOAT> normals;
    std::vector<FLOAT> textureCoordinates;

    char lineType;
    FLOAT x, y, z;
    UINT i1, i2, i3;

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

    return std::unique_ptr<Mesh>{new MeshImpl(application, MeshImpl::MeshType::TRIANGLE_STRIP,
                                              std::move(vertices), std::move(indices),
                                              std::move(normals), std::move(textureCoordinates))};
}
} // namespace DXD

MeshImpl::MeshImpl(DXD::Application &application, MeshType meshType,
                   std::vector<FLOAT> &&vertices, std::vector<UINT> &&indices,
                   std::vector<FLOAT> &&normals, std::vector<FLOAT> &&textureCoordinates)
    : application(*static_cast<ApplicationImpl *>(&application)),
      meshType(meshType),
      vertices(std::move(vertices)),
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
    const UINT verticesCount = static_cast<UINT>(vertices.size() / 3);
    const UINT vertexSize = 3 * sizeof(FLOAT);
    this->vertexBuffer = std::make_unique<VertexBuffer>(device, commandList, vertices.data(), verticesCount, vertexSize);
    this->indexBuffer = std::make_unique<IndexBuffer>(device, commandList, indices.data(), static_cast<UINT>(indices.size()), static_cast<UINT>(sizeof(UINT)));
    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    std::vector<CommandList *> commandLists{&commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);
}
