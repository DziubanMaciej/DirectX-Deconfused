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
    ID3D12DevicePtr device = application.getDevice();

    auto &commandQueue = application.getDirectCommandQueue();
    CommandList commandList{commandQueue.getCommandAllocatorManager(), nullptr};

    {
        const UINT verticesSize = static_cast<UINT>(vertices.size()) * sizeof(FLOAT);
        vertexBuffer = std::make_unique<Resource>(device, D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE, verticesSize, D3D12_RESOURCE_STATE_COPY_DEST, nullptr);
        Resource vertexBufferUploadHeap(device, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, verticesSize, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);

        // Copy the triangle data to the vertex buffer.
        D3D12_SUBRESOURCE_DATA vertexData = {};
        vertexData.pData = vertices.data();
        vertexData.RowPitch = verticesSize;
        vertexData.SlicePitch = verticesSize;

        UpdateSubresources<1>(commandList.getCommandList().Get(), vertexBuffer->getResource().Get(), vertexBufferUploadHeap.getResource().Get(), 0, 0, 1, &vertexData);
        commandList.transitionBarrierSingle(vertexBuffer->getResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        commandList.addUsedResource(vertexBufferUploadHeap.getResource());

        // Initialize the vertex buffer view.
        vertexBufferView.BufferLocation = vertexBuffer->getResource()->GetGPUVirtualAddress();
        vertexBufferView.StrideInBytes = 3 * sizeof(FLOAT); // TODO
        vertexBufferView.SizeInBytes = verticesSize;
    }

    {
        const UINT indicesSize = static_cast<UINT>(indices.size()) * sizeof(UINT);
        indexBuffer = std::make_unique<Resource>(device, D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE, indicesSize, D3D12_RESOURCE_STATE_COPY_DEST, nullptr);
        Resource indexBufferUploadHeap(device, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, indicesSize, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);

        // Copy the triangle data to the index buffer.
        D3D12_SUBRESOURCE_DATA indexData = {};
        indexData.pData = indices.data();
        indexData.RowPitch = indicesSize;
        indexData.SlicePitch = indicesSize;

        UpdateSubresources<1>(commandList.getCommandList().Get(), indexBuffer->getResource().Get(), indexBufferUploadHeap.getResource().Get(), 0, 0, 1, &indexData);
        commandList.transitionBarrierSingle(indexBuffer->getResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
        commandList.addUsedResource(indexBufferUploadHeap.getResource());

        // Initialize the index buffer view.
        indexBufferView.BufferLocation = indexBuffer->getResource()->GetGPUVirtualAddress();
        indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        indexBufferView.SizeInBytes = indicesSize;
    }

    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    std::vector<CommandList *> commandLists{&commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);
}
