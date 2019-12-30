#pragma once

#include "CommandList/CommandList.h"
#include "CommandList/CommandQueue.h"
#include "Utility/GpuDependency.h"

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <ExternalHeaders/Wrappers/d3dx12.h>
#include <mutex>

class ApplicationImpl;
class CommandList;
class AcquiredD2DWrappedResource;

class Resource : DXD::NonCopyable {
public:
    constexpr static UINT maxSubresourcesCount = 10;

    struct ResourceState {
        struct BarriersCreationData {
            BarriersCreationData(Resource &resource) : resource(resource) {}
            Resource &resource;
            D3D12_RESOURCE_BARRIER barriers[maxSubresourcesCount] = {};
            UINT barriersCount = 0u;
        };

        explicit ResourceState(D3D12_RESOURCE_STATES state) : resourceState(state) {}
        ResourceState &operator=(D3D12_RESOURCE_STATES state);
        D3D12_RESOURCE_STATES resourceState;
        bool hasSubresourceSpecificState = false;
        D3D12_RESOURCE_STATES subresourcesStates[maxSubresourcesCount] = {};

        D3D12_RESOURCE_STATES getState(UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) const;
        void setState(D3D12_RESOURCE_STATES state, UINT subresource, BarriersCreationData *outBarriers = nullptr);
        bool areAllSubresourcesInState(D3D12_RESOURCE_STATES state) const;
    };

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
    auto getSubresourcesCount() const { return subresourcesCount; }
    void reset();
    void setResource(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES state, UINT subresourcesCount);

    // Gpu dependency functions
    bool isWaitingForGpuDependencies();
    void addGpuDependency(CommandQueue &queue, uint64_t fenceValue);

    // Descriptors
    void createNullSrv(D3D12_SHADER_RESOURCE_VIEW_DESC *desc);
    void createCbv(D3D12_CONSTANT_BUFFER_VIEW_DESC *desc);
    void createSrv(D3D12_SHADER_RESOURCE_VIEW_DESC *desc);
    void createUav(D3D12_UNORDERED_ACCESS_VIEW_DESC *desc);
    void createDsv(D3D12_DEPTH_STENCIL_VIEW_DESC *desc);
    void createRtv(D3D12_RENDER_TARGET_VIEW_DESC *desc);
    D3D12_CPU_DESCRIPTOR_HANDLE getCbv() const { return descriptorsCbvSrvUav.getCpuHandle(cbvIndexInAllocation); }
    D3D12_CPU_DESCRIPTOR_HANDLE getSrv() const { return descriptorsCbvSrvUav.getCpuHandle(srvIndexInAllocation); }
    D3D12_CPU_DESCRIPTOR_HANDLE getUav() const { return descriptorsCbvSrvUav.getCpuHandle(uavIndexInAllocation); }
    D3D12_CPU_DESCRIPTOR_HANDLE getDsv() const { return descriptorsDsv.getCpuHandle(); }
    D3D12_CPU_DESCRIPTOR_HANDLE getRtv() const { return descriptorsRtv.getCpuHandle(); }

protected:
    static ID3D12ResourcePtr createResource(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags,
        const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState,
        const D3D12_CLEAR_VALUE *pOptimizedClearValue);

    // Gpu dependency functions
    void waitOnGpuForGpuUpload(CommandQueue &queue);
    void uploadToGPU(ApplicationImpl &application, const void *data, UINT rowPitch, UINT slicePitch);
    void recordGpuUploadCommands(ID3D12DevicePtr device, CommandList &commandList, const void *data, UINT rowPitch, UINT slicePitch);

private:
    // helpers
    void ensureDescriptorAllocationIsPresent(DescriptorAllocation &allocation, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count);

    // state accessors, should be used only by classes making transitions, hence the friend declarations
    const ResourceState &getState() const { return state; }
    void setState(const ResourceState &state) { this->state = state; }
    friend class CommandList;
    friend class AcquiredD2DWrappedResource;

    // General data
    ResourceState state;
    ID3D12ResourcePtr resource = {};
    UINT subresourcesCount = 1u;
    GpuDependencies gpuDependencies = {};

    // Gpu dependencies
    std::mutex gpuDependenciesLock = {};

    // Descriptors
    DescriptorAllocation descriptorsCbvSrvUav = {};
    DescriptorAllocation descriptorsDsv = {};
    DescriptorAllocation descriptorsRtv = {};
    constexpr static UINT srvIndexInAllocation = 0;
    constexpr static UINT cbvIndexInAllocation = 1;
    constexpr static UINT uavIndexInAllocation = 2;
    constexpr static UINT cbvSrvUavDescriptorsCount = 3;
};
