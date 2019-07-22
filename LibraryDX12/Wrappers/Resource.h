#pragma once

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include "DXD/ExternalHeadersWrappers/d3dx12.h"

class ApplicationImpl;
class CommandList;

class Resource : DXD::NonCopyable {
public:
    explicit Resource(ID3D12ResourcePtr resource);
    explicit Resource(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue);
    explicit Resource(ID3D12DevicePtr device, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlags, const int bufferSize, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue);
    Resource(Resource &&other) = default;
    Resource &operator=(Resource &&other) = default;
    auto &getResource() { return resource; };
    void setResource(ID3D12ResourcePtr resource) { this->resource = resource; };

private:
    void create(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue);

protected:
    void uploadToGPU(ApplicationImpl &application, const void *data, D3D12_RESOURCE_STATES resourceStateToTransition);
    void recordGpuUploadCommands(ID3D12DevicePtr device, CommandList &commandList, const void *data, D3D12_RESOURCE_STATES resourceStateToTransition);

    ID3D12ResourcePtr resource;
};
