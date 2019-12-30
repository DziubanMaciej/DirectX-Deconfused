#include "DescriptorAllocation.h"

#include "Descriptor/DescriptorHeap.h"

#include <cassert>

DescriptorAllocation::DescriptorAllocation()
    : heap(nullptr),
      offsetInHeap(),
      handlesCount(),
      cpuHandle(),
      gpuHandle(),
      isGpuVisible(false),
      descriptorIncrementSize() {
}

DescriptorAllocation::DescriptorAllocation(DescriptorHeap &heap, DescriptorHeap::FreeListOffset offsetInHeap,
                                           DescriptorHeap::FreeListSize handlesCount, D3D12_CPU_DESCRIPTOR_HANDLE heapBaseHandle, UINT descriptorIncrementSize)
    : heap(&heap),
      offsetInHeap(offsetInHeap),
      handlesCount(handlesCount),
      cpuHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE{heapBaseHandle, static_cast<INT>(offsetInHeap), descriptorIncrementSize}),
      gpuHandle(),
      isGpuVisible(false),
      descriptorIncrementSize(descriptorIncrementSize) {
}

DescriptorAllocation::DescriptorAllocation(DescriptorHeap &heap, DescriptorHeap::FreeListOffset offsetInHeap,
                                           DescriptorHeap::FreeListSize handlesCount, D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapBaseHandle,
                                           D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapBaseHandle, UINT descriptorIncrementSize)
    : heap(&heap),
      offsetInHeap(offsetInHeap),
      handlesCount(handlesCount),
      cpuHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE{cpuHeapBaseHandle, static_cast<INT>(offsetInHeap), descriptorIncrementSize}),
      gpuHandle(CD3DX12_GPU_DESCRIPTOR_HANDLE{gpuHeapBaseHandle, static_cast<INT>(offsetInHeap), descriptorIncrementSize}),
      isGpuVisible(true),
      descriptorIncrementSize(descriptorIncrementSize) {
}

DescriptorAllocation::DescriptorAllocation(DescriptorAllocation &&other) {
    *this = std::move(other);
}

DescriptorAllocation &DescriptorAllocation::operator=(DescriptorAllocation &&other) {
    deallocate();

    heap = std::move(other.heap);
    offsetInHeap = std::move(other.offsetInHeap);
    handlesCount = std::move(other.handlesCount);
    cpuHandle = std::move(other.cpuHandle);
    gpuHandle = std::move(other.gpuHandle);
    isGpuVisible = std::move(other.isGpuVisible);
    descriptorIncrementSize = std::move(other.descriptorIncrementSize);

    other.heap = nullptr;
    return *this;
}

DescriptorAllocation::~DescriptorAllocation() {
    deallocate();
}

void DescriptorAllocation::deallocate() {
    if (!isNull()) {
        heap->deallocate(*this);
        heap = nullptr;
    }
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::getCpuHandle() const {
    assert(!isNull());
    return CD3DX12_CPU_DESCRIPTOR_HANDLE{cpuHandle};
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::getCpuHandle(UINT offset) const {
    assert(!isNull());
    return CD3DX12_CPU_DESCRIPTOR_HANDLE{cpuHandle, static_cast<INT>(offset), descriptorIncrementSize};
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorAllocation::getGpuHandle() const {
    assert(!isNull());
    assert(isGpuVisible);
    return CD3DX12_GPU_DESCRIPTOR_HANDLE{gpuHandle};
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorAllocation::getGpuHandle(UINT offset) const {
    assert(!isNull());
    assert(isGpuVisible);
    return CD3DX12_GPU_DESCRIPTOR_HANDLE{gpuHandle, static_cast<INT>(offset), descriptorIncrementSize};
}
