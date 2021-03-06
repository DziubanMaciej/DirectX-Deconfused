#pragma once

#include "CommandList/CommandList.h"
#include "Resource/Resource.h"

#include <DXD/ExternalHeadersWrappers/d3d12.h>

// ---------------------------------------------------------------------------------------------------------------- Template class handling index or vertex buffer

class VertexOrIndexBuffer : public Resource {
public:
    VertexOrIndexBuffer(ID3D12DevicePtr device, CommandList &commandList, const void *data, UINT size)
        : Resource(device, D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE, size, D3D12_RESOURCE_STATE_COPY_DEST, nullptr) {
        const auto desc = getResource()->GetDesc();
        recordGpuUploadCommands(device, commandList, data, static_cast<UINT>(desc.Width), desc.Height);
    }
};

class VertexBuffer : public VertexOrIndexBuffer {
public:
    VertexBuffer(ID3D12DevicePtr device, CommandList &commandList, const void *data, UINT verticesCount, UINT vertexSize)
        : VertexOrIndexBuffer(device, commandList, data, verticesCount * vertexSize) {
        view.BufferLocation = getResource()->GetGPUVirtualAddress();
        view.SizeInBytes = verticesCount * vertexSize;
        view.StrideInBytes = vertexSize;
    }
    const auto &getView() const { return view; }

private:
    D3D12_VERTEX_BUFFER_VIEW view = {};
};

class IndexBuffer : public VertexOrIndexBuffer {
public:
    IndexBuffer(ID3D12DevicePtr device, CommandList &commandList, const void *data, UINT indicesCount)
        : VertexOrIndexBuffer(device, commandList, data, indicesCount * sizeof(UINT)) {
        view.BufferLocation = getResource()->GetGPUVirtualAddress();
        view.SizeInBytes = indicesCount * sizeof(UINT);
        view.Format = DXGI_FORMAT_R32_UINT;
    }
    const auto &getView() const { return view; }

private:
    D3D12_INDEX_BUFFER_VIEW view = {};
};
