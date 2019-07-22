#pragma once

#include "Wrappers/CommandList.h"
#include "Wrappers/Resource.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"

// ---------------------------------------------------------------------------------------------------------------- Template class handling index or vertex buffer
template <typename _ViewType>
class VertexOrIndexBuffer : public Resource {
public:
    VertexOrIndexBuffer(ID3D12DevicePtr device, CommandList &commandList, const void *data, UINT numElements, UINT elementSize)
        : Resource(device, D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE, numElements * elementSize, D3D12_RESOURCE_STATE_COPY_DEST, nullptr) {
        recordGpuUploadCommands(device, commandList, data, numElements * elementSize, getDestinationResourceState());
        populateView(numElements, elementSize);
    }

    auto getView() const { return view; }
private:
    void populateView(UINT numElements, UINT elementSize);
    static D3D12_RESOURCE_STATES getDestinationResourceState();

    _ViewType view = {};
};

// ---------------------------------------------------------------------------------------------------------------- Instantation for vertex buffer
template <>
inline void VertexOrIndexBuffer<D3D12_VERTEX_BUFFER_VIEW>::populateView(UINT numElements, UINT elementSize) {
    view.BufferLocation = resource->GetGPUVirtualAddress();
    view.SizeInBytes = elementSize * numElements;
    view.StrideInBytes = elementSize;
}
inline D3D12_RESOURCE_STATES VertexOrIndexBuffer<D3D12_VERTEX_BUFFER_VIEW>::getDestinationResourceState() {
    return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
}
using VertexBuffer = VertexOrIndexBuffer<D3D12_VERTEX_BUFFER_VIEW>;

// ---------------------------------------------------------------------------------------------------------------- Instantation for index buffer
template <>
inline void VertexOrIndexBuffer<D3D12_INDEX_BUFFER_VIEW>::populateView(UINT numElements, UINT elementSize) {
    view.BufferLocation = resource->GetGPUVirtualAddress();
    view.SizeInBytes = elementSize * numElements;
    view.Format = DXGI_FORMAT_R32_UINT;;
}
inline D3D12_RESOURCE_STATES VertexOrIndexBuffer<D3D12_INDEX_BUFFER_VIEW>::getDestinationResourceState() {
    return D3D12_RESOURCE_STATE_INDEX_BUFFER;
}
using IndexBuffer = VertexOrIndexBuffer<D3D12_INDEX_BUFFER_VIEW>;
