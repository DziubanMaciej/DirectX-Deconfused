#include "Resource.h"

#include "Utility/ThrowIfFailed.h"

Resource::Resource(ID3D12ResourcePtr resource) : resource(resource) {
}

Resource::Resource(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue) {
    create(device, pHeapProperties, heapFlags, pDesc, initialResourceState, pOptimizedClearValue);
}

Resource::Resource(ID3D12DevicePtr device, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlags, const int bufferSize, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue) {
    create(device, &CD3DX12_HEAP_PROPERTIES(heapType), heapFlags, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize), initialResourceState, pOptimizedClearValue);
}

void Resource::create(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue) {
    throwIfFailed(device->CreateCommittedResource(
        pHeapProperties,
        heapFlags,
        pDesc,
        initialResourceState,
        pOptimizedClearValue,
        IID_PPV_ARGS(&resource)));
}
