#include "Resource.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "Utility/ThrowIfFailed.h"

#include <cassert>

Resource::Resource(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES state)
    : resource(resource),
      state(state),
      descriptorsCbvSrvUav(ApplicationImpl::getInstance().getDescriptorController().allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3)),
      descriptorsDsv(ApplicationImpl::getInstance().getDescriptorController().allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1)),
      descriptorsRtv(ApplicationImpl::getInstance().getDescriptorController().allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1)) {}

Resource::Resource() : Resource(nullptr, D3D12_RESOURCE_STATE_COMMON) {}

Resource::Resource(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags,
                   const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState,
                   const D3D12_CLEAR_VALUE *pOptimizedClearValue)
    : Resource(createResource(device, pHeapProperties, heapFlags, pDesc, initialResourceState, pOptimizedClearValue), initialResourceState) {}

Resource::Resource(ID3D12DevicePtr device, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlags, UINT64 bufferSize,
                   D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue)
    : Resource(createResource(device, &CD3DX12_HEAP_PROPERTIES(heapType), heapFlags, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize), initialResourceState, pOptimizedClearValue), initialResourceState) {}

void Resource::reset() {
    gpuUploadData.reset();
    resource.Reset();
}

void Resource::setResource(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES state, UINT subresourcesCount) {
    assert(subresourcesCount < maxSubresourcesCount);
    this->resource = resource;
    this->state = state;
    this->subresourcesCount = subresourcesCount;
}

bool Resource::isUploadInProgress() {
    if (gpuUploadData != nullptr && gpuUploadData->uploadingQueue.getFence().isComplete(gpuUploadData->uploadFence)) {
        gpuUploadData.reset();
    }
    return gpuUploadData != nullptr;
}

void Resource::registerUpload(CommandQueue &uploadingQueue, uint64_t uploadFence) {
    assert(!Resource::isUploadInProgress()); // only check GPU upload, not CPU loading
    this->gpuUploadData = std::make_unique<GpuUploadData>(uploadingQueue, uploadFence);
}

void Resource::createCbv(D3D12_CONSTANT_BUFFER_VIEW_DESC *desc) {
    ID3D12DevicePtr device = {};
    throwIfFailed(resource->GetDevice(IID_PPV_ARGS(&device)));
    device->CreateConstantBufferView(desc, descriptorsCbvSrvUav.getCpuHandle(0));
}

void Resource::createSrv(D3D12_SHADER_RESOURCE_VIEW_DESC *desc) {
    ID3D12DevicePtr device = {};
    throwIfFailed(resource->GetDevice(IID_PPV_ARGS(&device)));
    device->CreateShaderResourceView(resource.Get(), desc, descriptorsCbvSrvUav.getCpuHandle(1));
}

void Resource::createUav(D3D12_UNORDERED_ACCESS_VIEW_DESC *desc) {
    ID3D12DevicePtr device = {};
    throwIfFailed(resource->GetDevice(IID_PPV_ARGS(&device)));
    device->CreateUnorderedAccessView(resource.Get(), nullptr, desc, descriptorsCbvSrvUav.getCpuHandle(2));
}

void Resource::createDsv(D3D12_DEPTH_STENCIL_VIEW_DESC *desc) {
    ID3D12DevicePtr device = {};
    throwIfFailed(resource->GetDevice(IID_PPV_ARGS(&device)));
    device->CreateDepthStencilView(resource.Get(), desc, descriptorsDsv.getCpuHandle());
}

void Resource::createRtv(D3D12_RENDER_TARGET_VIEW_DESC *desc) {
    ID3D12DevicePtr device = {};
    throwIfFailed(resource->GetDevice(IID_PPV_ARGS(&device)));
    device->CreateRenderTargetView(resource.Get(), desc, descriptorsRtv.getCpuHandle());
}

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

void Resource::uploadToGPU(ApplicationImpl &application, const void *data, UINT rowPitch, UINT slicePitch) {
    assert(gpuUploadData == nullptr);

    CommandQueue &commandQueue = application.getCopyCommandQueue();

    // Record command list for GPU upload
    CommandList commandList{commandQueue};
    recordGpuUploadCommands(application.getDevice(), commandList, data, rowPitch, slicePitch);
    commandList.close();

    // Execute on GPU
    const auto fence = commandQueue.executeCommandListAndSignal(commandList);
    registerUpload(commandQueue, fence);
}

void Resource::recordGpuUploadCommands(ID3D12DevicePtr device, CommandList &commandList, const void *data, UINT rowPitch, UINT slicePitch) {
    assert(getState() & D3D12_RESOURCE_STATE_COPY_DEST);

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

void Resource::ResourceState::setState(D3D12_RESOURCE_STATES state, UINT subresource) {
    if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
        // Setting all subresources to the same state
        this->resourceState = state;
        this->hasSubresourceSpecificState = false;

    } else {
        // Setting state for only one subresource
        if (!this->hasSubresourceSpecificState) {
            std::fill_n(this->subresourcesStates, maxSubresourcesCount, this->resourceState);
            this->hasSubresourceSpecificState = true;
        }
        this->subresourcesStates[subresource] = state;
    };
}

D3D12_RESOURCE_STATES Resource::ResourceState::getState() const {
    assert(!this->hasSubresourceSpecificState);
    return this->resourceState;
}
