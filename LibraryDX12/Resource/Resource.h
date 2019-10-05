#pragma once

#include "CommandList/CommandList.h"
#include "CommandList/CommandQueue.h"

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <ExternalHeaders/Wrappers/d3dx12.h>

class ApplicationImpl;
class CommandList;
class AcquiredD2DWrappedResource;

class Resource : DXD::NonCopyable {
public:
    constexpr static UINT maxSubresourcesCount = 10;

    /// Base constructor initializing every member. It is called by every other constructor. It can also be used
    /// to wrap an existing ID3D12Resource, e.g. a back buffer allocated by SwapChain.
    explicit Resource(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES state);

    /// Creates empty Resource object to be filled later, used by subclasses. Does not perform GPU allocation
    explicit Resource();

    /// Allocates new resource
    explicit Resource(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags,
                      const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState,
                      const D3D12_CLEAR_VALUE *pOptimizedClearValue);

    /// Allocates new buffer resource
    explicit Resource(ID3D12DevicePtr device, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlags, UINT64 bufferSize,
                      D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue);
    virtual ~Resource() = default;
    Resource(Resource &&other) = default;
    Resource &operator=(Resource &&other) = default;

    // Accessors
    auto getResource() { return resource; };
    const auto getResource() const { return resource; }
    void reset();
    void setResource(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES state);

    // Gpu upload functions
    bool isUploadInProgress();
    void registerUpload(CommandQueue &uploadingQueue, uint64_t uploadFence);

    // Descriptors
    void createCbv(D3D12_CONSTANT_BUFFER_VIEW_DESC *desc);
    void createSrv(D3D12_SHADER_RESOURCE_VIEW_DESC *desc);
    void createUav(D3D12_UNORDERED_ACCESS_VIEW_DESC *desc);
    void createDsv(D3D12_DEPTH_STENCIL_VIEW_DESC *desc);
    void createRtv(D3D12_RENDER_TARGET_VIEW_DESC *desc);
    D3D12_CPU_DESCRIPTOR_HANDLE getCbv() const { return descriptorsCbvSrvUav.getCpuHandle(0); }
    D3D12_CPU_DESCRIPTOR_HANDLE getSrv() const { return descriptorsCbvSrvUav.getCpuHandle(1); }
    D3D12_CPU_DESCRIPTOR_HANDLE getUav() const { return descriptorsCbvSrvUav.getCpuHandle(2); }
    D3D12_CPU_DESCRIPTOR_HANDLE getDsv() const { return descriptorsDsv.getCpuHandle(); }
    D3D12_CPU_DESCRIPTOR_HANDLE getRtv() const { return descriptorsRtv.getCpuHandle(); }

protected:
    static ID3D12ResourcePtr createResource(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags,
                                            const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState,
                                            const D3D12_CLEAR_VALUE *pOptimizedClearValue);

    // Gpu upload functions
    struct GpuUploadData {
        GpuUploadData(CommandQueue &queue, uint64_t fence) : uploadingQueue(queue), uploadFence(fence) {}
        CommandQueue &uploadingQueue;
        uint64_t uploadFence;
    };
    void uploadToGPU(ApplicationImpl &application, const void *data, UINT rowPitch, UINT slicePitch);
    void recordGpuUploadCommands(ID3D12DevicePtr device, CommandList &commandList, const void *data, UINT rowPitch, UINT slicePitch);

private:
    // state accessors, should be used only by classes making transitions, hence the friend declarations
    void setState(D3D12_RESOURCE_STATES state, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
    D3D12_RESOURCE_STATES getState() const;
    friend class CommandList;
    friend class AcquiredD2DWrappedResource;

    // Data
    struct ResourceState {
        explicit ResourceState(D3D12_RESOURCE_STATES state) : resourceState(state) {}
        ResourceState &operator=(D3D12_RESOURCE_STATES state) {
            resourceState = state;
            hasSubresourceSpecificState = false;
            return *this;
        }
        D3D12_RESOURCE_STATES resourceState;
        bool hasSubresourceSpecificState = false;
        D3D12_RESOURCE_STATES subresourcesStates[maxSubresourcesCount] = {};
    } state;
    ID3D12ResourcePtr resource = {};
    std::unique_ptr<GpuUploadData> gpuUploadData = {};
    DescriptorAllocation descriptorsCbvSrvUav;
    DescriptorAllocation descriptorsDsv;
    DescriptorAllocation descriptorsRtv;
};
