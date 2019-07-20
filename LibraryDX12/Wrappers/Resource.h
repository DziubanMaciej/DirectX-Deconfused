#pragma once

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include "DXD/ExternalHeadersWrappers/d3dx12.h"

class Resource : DXD::NonCopyable {
public:
    explicit Resource(ID3D12DevicePtr device);
    Resource(Resource &&other) = default;
    Resource &operator=(Resource &&other) = default;

    void create(const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue);
    void create(D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlags, const int bufferSize, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue);
    auto getResource() { return resource; };

private:
    ID3D12ResourcePtr resource;
    ID3D12DevicePtr device;
};
