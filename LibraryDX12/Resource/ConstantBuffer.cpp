#include "ConstantBuffer.h"

#include "Application/ApplicationImpl.h"
#include "Descriptor/DescriptorController.h"
#include "Utility/ThrowIfFailed.h"

// ---------------------------------------------------------------------------------------- Creation

ConstantBuffer::ConstantBuffer(UINT size, UINT subbuffersCount)
    : Resource(ApplicationImpl::getInstance().getDevice(), D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE,
               getTotalBufferSize(size, subbuffersCount), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr),
      subbufferSize(getAlignedSubbufferSize(size)),
      totalSize(getTotalBufferSize(size, subbuffersCount)),
      subbuffersCount(subbuffersCount),
      mappedConstantBuffer(map(getResource())),
      stagingData(std::make_unique<uint8_t[]>(totalSize)),
      descriptors(ApplicationImpl::getInstance().getDescriptorController().allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, subbuffersCount)) {
    createDescriptors();
}

ConstantBuffer::~ConstantBuffer() {
    const CD3DX12_RANGE writeRange(0, 0); // Entire resource might have been written
    getResource()->Unmap(0, &writeRange);
}

// ---------------------------------------------------------------------------------------- Uploading

D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::uploadAndSwap() {
    const D3D12_CPU_DESCRIPTOR_HANDLE resultHandle = descriptors.getCpuHandle(currentSubbufferIndex);
    if (dirtyStagingData) {
        // Upload
        const auto offset = getSubbufferOffset(subbufferSize, currentSubbufferIndex);
        const auto destinationAddress = mappedConstantBuffer + offset;
        const auto sourceAddress = stagingData.get() + offset;
        memcpy_s(destinationAddress, subbufferSize, sourceAddress, subbufferSize);

        // Swap subbufer to the next one and clear the flag
        currentSubbufferIndex = (currentSubbufferIndex + 1) % subbuffersCount;
        dirtyStagingData = false;
    }

    // Return descriptor to uploaded memory
    return resultHandle;
}

// ---------------------------------------------------------------------------------------- Helpers

uint8_t *ConstantBuffer::map(ID3D12ResourcePtr &resource) {
    const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    void *mappedData = nullptr;
    throwIfFailed(resource->Map(0, &readRange, &mappedData));
    return reinterpret_cast<uint8_t *>(mappedData);
}

UINT64 ConstantBuffer::getAlignedSubbufferSize(UINT64 subbufferSize) {
    return MathHelper::alignUp<D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(subbufferSize);
}

UINT64 ConstantBuffer::getTotalBufferSize(UINT64 subbufferSize, UINT subbuffersCount) {
    return getSubbufferOffset(subbufferSize, subbuffersCount) + 100;
}

UINT64 ConstantBuffer::getSubbufferOffset(UINT64 subbufferSize, UINT subbufferIndex) {
    return getAlignedSubbufferSize(subbufferSize) * subbufferIndex;
}

void ConstantBuffer::createDescriptors() {
    auto device = ApplicationImpl::getInstance().getDevice();

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDescription = {};
    cbvDescription.SizeInBytes = static_cast<UINT>(subbufferSize);
    cbvDescription.BufferLocation = getResource()->GetGPUVirtualAddress();

    for (auto subbufferIndex = 0u; subbufferIndex < subbuffersCount; subbufferIndex++) {
        device->CreateConstantBufferView(&cbvDescription, descriptors.getCpuHandle(subbufferIndex));
        cbvDescription.BufferLocation += subbufferSize;
    }
}
