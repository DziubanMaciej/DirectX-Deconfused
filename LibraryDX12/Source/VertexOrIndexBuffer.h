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
        uploadToGPU(device, commandList, data, numElements * elementSize);
        populateView(numElements, elementSize);
    }

    auto getView() const { return view; }

    void uploadToGPU(ID3D12DevicePtr device, CommandList &commandList, const void *data, UINT bufferSize) {
        // Create buffer on upload heap
        Resource intermediateResource(device, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, bufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);

        // Transfer data through the upload heap to destination resource
        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = data;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = bufferSize;
        UpdateSubresources<1>(commandList.getCommandList().Get(), this->resource.Get(), intermediateResource.getResource().Get(), 0, 0, 1, &subresourceData);

        // Transition destination resource and make intermediateResource tracked so it's not deleted while being processed on the GPU
        commandList.transitionBarrierSingle(this->resource, D3D12_RESOURCE_STATE_COPY_DEST, getDestinationResourceState());
        commandList.addUsedResource(intermediateResource.getResource());
    }

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
