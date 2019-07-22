#include "Resource.h"

#include "Api/ApplicationImpl.h"
#include "Utility/ThrowIfFailed.h"
#include "Wrappers/CommandList.h"

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


void Resource::uploadToGPU(ApplicationImpl &application, const void *data, D3D12_RESOURCE_STATES resourceStateToTransition) {
    CommandQueue &commandQueue = application.getDirectCommandQueue();

    // Record command list for GPU upload
    CommandList commandList{ commandQueue.getCommandAllocatorManager(), nullptr };
    recordGpuUploadCommands(application.getDevice(), commandList, data, resourceStateToTransition);
    commandList.close();

    // Execute on GPU
    std::vector<CommandList *> commandLists{ &commandList };
    commandQueue.executeCommandListsAndSignal(commandLists);
}

void Resource::recordGpuUploadCommands(ID3D12DevicePtr device, CommandList &commandList, const void *data, D3D12_RESOURCE_STATES resourceStateToTransition) {
    // TODO make assertion that this->resource is in D3D12_RESOURCE_STATE_COPY_DEST state

    // Create buffer on upload heap
    Resource intermediateResource(device, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, GetRequiredIntermediateSize(resource.Get(), 0, 1), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);

    // Transfer data through the upload heap to destination resource
    const auto resourceDescription = resource->GetDesc();
    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData = data;
    subresourceData.RowPitch = resourceDescription.Width;
    subresourceData.SlicePitch = resourceDescription.Height;
    UpdateSubresources<1>(commandList.getCommandList().Get(), this->resource.Get(), intermediateResource.getResource().Get(), 0, 0, 1, &subresourceData);

    // Transition destination resource and make intermediateResource tracked so it's not deleted while being processed on the GPU
    commandList.transitionBarrierSingle(this->resource, D3D12_RESOURCE_STATE_COPY_DEST, resourceStateToTransition);
    commandList.addUsedResource(intermediateResource.getResource());
}
