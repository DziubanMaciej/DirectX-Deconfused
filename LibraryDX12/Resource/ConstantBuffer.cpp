#include "ConstantBuffer.h"

#include "Descriptor/DescriptorController.h"
#include "Utility/ThrowIfFailed.h"

ConstantBuffer::ConstantBuffer(ID3D12DevicePtr device, DescriptorController &descriptorController, UINT size)
    : Resource(device, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, BitHelper::alignUp<64 * 1024>(size), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr),
      size(size),
      mappedConstantBuffer(map(this->resource)),
      data(std::make_unique<uint8_t[]>(size)) {
    createDescriptor(device);
}

ConstantBuffer::~ConstantBuffer() {
    const CD3DX12_RANGE writeRange(0, 0); // Entire resource might have been written
    resource->Unmap(0, &writeRange);
}

void ConstantBuffer::upload() {
    memcpy_s(mappedConstantBuffer, size, data.get(), size);
}

void *ConstantBuffer::map(ID3D12ResourcePtr &resource) {
    const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    void *mappedData = nullptr;
    throwIfFailed(resource->Map(0, &readRange, &mappedData));
    return mappedData;
}

void ConstantBuffer::createDescriptor(ID3D12DevicePtr &device) {
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDescription = {};
    cbvDescription.BufferLocation = this->resource->GetGPUVirtualAddress();
    cbvDescription.SizeInBytes = BitHelper::alignUp<256>(size);
    createCbv(&cbvDescription);
}
