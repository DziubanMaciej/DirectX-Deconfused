#include "CpuDescriptorAllocation.h"

#include "Descriptor/CpuDescriptorHeap.h"

#include <cassert>

CpuDescriptorAllocation::CpuDescriptorAllocation(CpuDescriptorHeap &heap, CpuDescriptorHeap::FreeListOffset offsetInHeap,
                                                 CpuDescriptorHeap::FreeListSize handlesCount, D3D12_CPU_DESCRIPTOR_HANDLE heapBaseHandle, UINT descriptorIncrementSize)
    : heap(&heap),
      offsetInHeap(offsetInHeap),
      handlesCount(handlesCount),
      cpuHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE{heapBaseHandle, handlesCount, descriptorIncrementSize}),
      descriptorIncrementSize(descriptorIncrementSize) {
}

CpuDescriptorAllocation::CpuDescriptorAllocation(CpuDescriptorAllocation &&other) {
    *this = std::move(other);
}

CpuDescriptorAllocation &CpuDescriptorAllocation::operator=(CpuDescriptorAllocation &&other) {
    heap = std::move(other.heap);
    offsetInHeap = std::move(other.offsetInHeap);
    handlesCount = std::move(other.handlesCount);
    cpuHandle = std::move(other.cpuHandle);
    descriptorIncrementSize = std::move(other.descriptorIncrementSize);

    other.heap = nullptr;
    return *this;
}

CpuDescriptorAllocation::~CpuDescriptorAllocation() {
    if (heap != nullptr) {
        heap->deallocate(*this);
        heap = nullptr;
    }
}
