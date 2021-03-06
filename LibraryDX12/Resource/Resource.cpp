#include "Resource.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "Utility/ThrowIfFailed.h"

#include <cassert>

// ------------------------------------------------------------------------------------- Creation

Resource::Resource(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES state)
    : resource(resource),
      state(state) {}

Resource::Resource() : Resource(nullptr, D3D12_RESOURCE_STATE_COMMON) {}

Resource::Resource(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags,
                   const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState,
                   const D3D12_CLEAR_VALUE *pOptimizedClearValue)
    : Resource(createResource(device, pHeapProperties, heapFlags, pDesc, initialResourceState, pOptimizedClearValue), initialResourceState) {}

Resource::Resource(ID3D12DevicePtr device, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlags, UINT64 bufferSize,
                   D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue)
    : Resource(createResource(device, &CD3DX12_HEAP_PROPERTIES(heapType), heapFlags, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize), initialResourceState, pOptimizedClearValue), initialResourceState) {}

ID3D12ResourcePtr Resource::createResource(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags,
                                           const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue) {
    ID3D12ResourcePtr resource = {};
    throwIfFailed(device->CreateCommittedResource(
        pHeapProperties,
        heapFlags,
        pDesc,
        initialResourceState,
        pOptimizedClearValue,
        IID_PPV_ARGS(&resource)));
    return resource;
}

// ------------------------------------------------------------------------------------- Accessors

void Resource::reset() {
    std::lock_guard<std::mutex> gpuDependenciesLock{this->gpuDependenciesLock};
    gpuDependencies.reset();
    resource.Reset();
}

void Resource::setResource(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES state, UINT subresourcesCount) {
    assert(subresourcesCount <= maxSubresourcesCount);
    this->resource = resource;
    this->state = state;
    this->subresourcesCount = subresourcesCount;
}

// ------------------------------------------------------------------------------------- Gpu dependencies functions

bool Resource::isWaitingForGpuDependencies() {
    std::unique_lock<std::mutex> gpuDependenciesLock{this->gpuDependenciesLock};
    return !gpuDependencies.isComplete();
}

void Resource::addGpuDependency(CommandQueue &queue, uint64_t fenceValue) {
    std::lock_guard<std::mutex> gpuDependenciesLock{this->gpuDependenciesLock};
    gpuDependencies.add(queue, fenceValue);
}

void Resource::waitOnGpuForGpuUpload(CommandQueue &queue) {
    std::lock_guard<std::mutex> gpuDependenciesLock{this->gpuDependenciesLock};
    gpuDependencies.waitOnGpu(queue);
}

void Resource::uploadToGPU(ApplicationImpl &application, const void *data, UINT rowPitch, UINT slicePitch) {
    CommandQueue &commandQueue = application.getCopyCommandQueue();

    // Record command list for GPU upload
    CommandList commandList{commandQueue};
    recordGpuUploadCommands(application.getDevice(), commandList, data, rowPitch, slicePitch);
    commandList.close();

    // Execute on GPU
    const auto fence = commandQueue.executeCommandListAndSignal(commandList);
    addGpuDependency(commandQueue, fence);
}

void Resource::recordGpuUploadCommands(ID3D12DevicePtr device, CommandList &commandList, const void *data, UINT rowPitch, UINT slicePitch) {
    assert(getState().areAllSubresourcesInState(D3D12_RESOURCE_STATE_COPY_DEST));

    // Create buffer on upload heap
    Resource intermediateResource(device, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, GetRequiredIntermediateSize(resource.Get(), 0, 1), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);

    // Transfer data through the upload heap to destination resource
    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData = data;
    subresourceData.RowPitch = rowPitch;
    subresourceData.SlicePitch = slicePitch;
    UpdateSubresources<1>(commandList.getCommandList().Get(), this->resource.Get(), intermediateResource.getResource().Get(), 0, 0, 1, &subresourceData);

    // Make intermediateResource tracked so it's not deleted while still being processed on the GPU
    commandList.addUsedResource(intermediateResource.getResource());
}

// ------------------------------------------------------------------------------------- Descriptors

void Resource::ensureDescriptorAllocationIsPresent(DescriptorAllocation &allocation, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count) {
    if (allocation.isNull()) {
        allocation = ApplicationImpl::getInstance().getDescriptorController().allocateCpu(type, count);
    }
}

void Resource::createNullSrv(D3D12_SHADER_RESOURCE_VIEW_DESC *desc) {
    ensureDescriptorAllocationIsPresent(descriptorsCbvSrvUav, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cbvSrvUavDescriptorsCount);
    ApplicationImpl::getInstance().getDevice()->CreateShaderResourceView(resource.Get(), desc, descriptorsCbvSrvUav.getCpuHandle(srvIndexInAllocation));
}

void Resource::createCbv(D3D12_CONSTANT_BUFFER_VIEW_DESC *desc) {
    ensureDescriptorAllocationIsPresent(descriptorsCbvSrvUav, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cbvSrvUavDescriptorsCount);
    ApplicationImpl::getInstance().getDevice()->CreateConstantBufferView(desc, descriptorsCbvSrvUav.getCpuHandle(cbvIndexInAllocation));
}

void Resource::createSrv(D3D12_SHADER_RESOURCE_VIEW_DESC *desc) {
    ensureDescriptorAllocationIsPresent(descriptorsCbvSrvUav, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cbvSrvUavDescriptorsCount);
    ApplicationImpl::getInstance().getDevice()->CreateShaderResourceView(resource.Get(), desc, descriptorsCbvSrvUav.getCpuHandle(srvIndexInAllocation));
}

void Resource::createUav(D3D12_UNORDERED_ACCESS_VIEW_DESC *desc) {
    ensureDescriptorAllocationIsPresent(descriptorsCbvSrvUav, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, cbvSrvUavDescriptorsCount);
    ApplicationImpl::getInstance().getDevice()->CreateUnorderedAccessView(resource.Get(), nullptr, desc, descriptorsCbvSrvUav.getCpuHandle(uavIndexInAllocation));
}

void Resource::createDsv(D3D12_DEPTH_STENCIL_VIEW_DESC *desc) {
    ensureDescriptorAllocationIsPresent(descriptorsDsv, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
    ApplicationImpl::getInstance().getDevice()->CreateDepthStencilView(resource.Get(), desc, descriptorsDsv.getCpuHandle());
}

void Resource::createRtv(D3D12_RENDER_TARGET_VIEW_DESC *desc) {
    ensureDescriptorAllocationIsPresent(descriptorsRtv, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);
    ApplicationImpl::getInstance().getDevice()->CreateRenderTargetView(resource.Get(), desc, descriptorsRtv.getCpuHandle());
}

// ------------------------------------------------------------------------------------- Resource::ResourceState class

Resource::ResourceState &Resource::ResourceState::operator=(D3D12_RESOURCE_STATES state) {
    resourceState = state;
    hasSubresourceSpecificState = false;
    return *this;
}

void Resource::ResourceState::setState(D3D12_RESOURCE_STATES state, UINT subresource, BarriersCreationData *outBarriers) {
    // Helper lambda function to avoid duplication
    auto addBarrier = [this, outBarriers, state](UINT subresource) {
        const auto currentState = getState(subresource);
        if (currentState != state) {
            auto &barrier = outBarriers->barriers[outBarriers->barriersCount++];
            barrier = CD3DX12_RESOURCE_BARRIER::Transition(outBarriers->resource.getResource().Get(), currentState, state, subresource);
        }
    };

    // Setting all subresources to the same state
    if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
        // Generate transition barriers
        if (outBarriers) {
            if (this->hasSubresourceSpecificState) {
                for (auto currentSubresource = 0u; currentSubresource < outBarriers->resource.getSubresourcesCount(); currentSubresource++) {
                    addBarrier(currentSubresource);
                }
            } else {
                addBarrier(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
            }
        }
        // Update data
        this->resourceState = state;
        this->hasSubresourceSpecificState = false;

    }
    // Setting state for only one subresource
    else {
        // Generate transition barrier
        if (outBarriers) {
            addBarrier(subresource);
        }

        // Set all subresources to correct state
        if (!this->hasSubresourceSpecificState) {
            std::fill_n(this->subresourcesStates, Resource::maxSubresourcesCount, this->resourceState);
            this->hasSubresourceSpecificState = true;
        }

        // Update target subresource
        this->subresourcesStates[subresource] = state;
    };
}

D3D12_RESOURCE_STATES Resource::ResourceState::getState(UINT subresource) const {
    if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
        assert(!this->hasSubresourceSpecificState);
        return this->resourceState;
    } else {
        if (hasSubresourceSpecificState) {
            return this->subresourcesStates[subresource];
        }
        return this->resourceState;
    }
}

bool Resource::ResourceState::areAllSubresourcesInState(D3D12_RESOURCE_STATES state) const {
    return !hasSubresourceSpecificState && (resourceState == state);
}
