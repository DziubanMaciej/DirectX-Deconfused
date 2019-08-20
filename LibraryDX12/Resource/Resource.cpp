#include "Resource.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "Utility/ThrowIfFailed.h"

#include <cassert>

Resource::Resource(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES state)
    : resource(resource),
      state(state) {}

Resource::Resource(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags,
                   const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState,
                   const D3D12_CLEAR_VALUE *pOptimizedClearValue)
    : resource(createResource(device, pHeapProperties, heapFlags, pDesc, state, pOptimizedClearValue)),
      state(initialResourceState) {}

Resource::Resource(ID3D12DevicePtr device, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlags, UINT64 bufferSize,
                   D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue)
    : resource(createResource(device, &CD3DX12_HEAP_PROPERTIES(heapType), heapFlags, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize), state, pOptimizedClearValue)),
      state(initialResourceState) {}

bool Resource::isUploadInProgress() {
    if (gpuUploadData != nullptr && gpuUploadData->uploadingQueue.getFence().isComplete(gpuUploadData->uploadFence)) {
        gpuUploadData.reset();
    }
    return gpuUploadData != nullptr;
}

void Resource::registerUpload(CommandQueue &uploadingQueue, uint64_t uploadFence) {
    assert(!isUploadInProgress());
    this->gpuUploadData = std::make_unique<GpuUploadData>(uploadingQueue, uploadFence);
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
    CommandList commandList{application.getDescriptorController(), commandQueue.getCommandAllocatorController(), nullptr};
    recordGpuUploadCommands(application.getDevice(), commandList, data, rowPitch, slicePitch);
    commandList.close();

    // Execute on GPU
    std::vector<CommandList *> commandLists{&commandList};
    const auto fence = commandQueue.executeCommandListsAndSignal(commandLists);
    registerUpload(commandQueue, fence);
}

void Resource::recordGpuUploadCommands(ID3D12DevicePtr device, CommandList &commandList, const void *data, UINT rowPitch, UINT slicePitch) {
    assert(state & D3D12_RESOURCE_STATE_COPY_DEST);

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
