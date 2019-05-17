#include "MeshImpl.h"
#include "ApplicationImpl.h"
#include "CommandList.h"
#include "Utility/ThrowIfFailed.h"
#include <fstream>

namespace DXD {
std::unique_ptr<Mesh> Mesh::create(Application &application) {
    return std::unique_ptr<Mesh>{new MeshImpl(application)};
}
} // namespace DXD

MeshImpl::MeshImpl(DXD::Application &application)
    : application(*static_cast<ApplicationImpl *>(&application)) {
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

    inputFile.close();

    meshType = MeshType::TRIANGLE_STRIP;

    uploadToGPU();

    return 0;
}

void MeshImpl::uploadToGPU() {
    ID3D12DevicePtr device = application.getDevice();

    auto &commandQueue = application.getDirectCommandQueue();
    CommandList commandList{commandQueue.getCommandAllocatorManager(), nullptr};

    ID3D12ResourcePtr vertexBufferUploadHeap;
    ID3D12ResourcePtr indexBufferUploadHeap;

    {
        const size_t verticesSize = vertices.size() * sizeof(FLOAT);

        throwIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(verticesSize),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&vertexBuffer)));

        throwIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(verticesSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertexBufferUploadHeap)));

        // Copy the triangle data to the vertex buffer.
        D3D12_SUBRESOURCE_DATA vertexData = {};
        vertexData.pData = vertices.data();
        vertexData.RowPitch = verticesSize;
        vertexData.SlicePitch = verticesSize;

        UpdateSubresources<1>(commandList.getCommandList().Get(), vertexBuffer.Get(), vertexBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
        commandList.transitionBarrierSingle(vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        commandList.addUsedResource(vertexBufferUploadHeap);

        // Initialize the vertex buffer view.
        vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.StrideInBytes = 3 * sizeof(FLOAT); // TODO
        vertexBufferView.SizeInBytes = verticesSize;
    }

    {
        const size_t indicesSize = indices.size() * sizeof(UINT);

        throwIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(indicesSize),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&indexBuffer)));

        throwIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(indicesSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&indexBufferUploadHeap)));

        // Copy the triangle data to the index buffer.
        D3D12_SUBRESOURCE_DATA indexData = {};
        indexData.pData = indices.data();
        indexData.RowPitch = indicesSize;
        indexData.SlicePitch = indicesSize;

        UpdateSubresources<1>(commandList.getCommandList().Get(), indexBuffer.Get(), indexBufferUploadHeap.Get(), 0, 0, 1, &indexData);
        commandList.transitionBarrierSingle(indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
        commandList.addUsedResource(indexBufferUploadHeap);

        // Initialize the index buffer view.
        indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
        indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        indexBufferView.SizeInBytes = indicesSize;
    }

    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    std::vector<CommandList *> commandLists{&commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);
}
