#pragma once

#include "CommandList/CommandList.h"
#include "CommandList/CommandQueue.h"

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include "DXD/ExternalHeadersWrappers/d3dx12.h"

class ApplicationImpl;
class CommandList;

class Resource : DXD::NonCopyable {
public:
    explicit Resource() {}
    explicit Resource(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES state);
    explicit Resource(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue);
    explicit Resource(ID3D12DevicePtr device, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlags, UINT64 bufferSize, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue);
    virtual ~Resource() = default;

    Resource(Resource &&other) = default;
    Resource &operator=(Resource &&other) = default;

    auto &getResource() { return resource; };
    const auto &getResource() const { return resource; }
    void setResource(ID3D12ResourcePtr resource) { this->resource = resource; };
    D3D12_RESOURCE_STATES getState() { return state; }

    // Gpu upload functions
    virtual bool isUploadInProgress();
    void registerUpload(CommandQueue &uploadingQueue, uint64_t uploadFence);

protected:
    void create(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue);

    // Gpu upload functions
    struct GpuUploadData {
        GpuUploadData(CommandQueue &queue, uint64_t fence) : uploadingQueue(queue), uploadFence(fence) {}
        CommandQueue &uploadingQueue;
        uint64_t uploadFence;
    };
    void uploadToGPU(ApplicationImpl &application, const void *data, UINT rowPitch, UINT slicePitch);
    void recordGpuUploadCommands(ID3D12DevicePtr device, CommandList &commandList, const void *data, UINT rowPitch, UINT slicePitch);

    // state accessors, should be used only by CommandList
    D3D12_RESOURCE_STATES getState() const { return state; }
    void setState(D3D12_RESOURCE_STATES state) { this->state = state; }
    friend class CommandList;

    // Data
    D3D12_RESOURCE_STATES state = {};
    ID3D12ResourcePtr resource = {};
    std::unique_ptr<GpuUploadData> gpuUploadData = {};
};
