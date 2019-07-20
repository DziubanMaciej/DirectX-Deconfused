#include "Resource.h"

#include "Utility/ThrowIfFailed.h"

Resource::Resource(ID3D12DevicePtr device) { this->device = device; }

void Resource::create(const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue) {
    throwIfFailed(device->CreateCommittedResource(
        pHeapProperties,
        heapFlags,
        pDesc,
        initialResourceState,
        pOptimizedClearValue,
        IID_PPV_ARGS(&resource)));
}

void Resource::create(D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlag, const int bufferSize, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue) {
    create(&CD3DX12_HEAP_PROPERTIES(heapType), heapFlag, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize), initialResourceState, pOptimizedClearValue);
}
