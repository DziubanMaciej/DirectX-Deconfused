#include "DescriptorController.h"

#include "Descriptor/DescriptorAllocation.h"
#include "Utility/ThrowIfFailed.h"

#include <cassert>

DescriptorController::DescriptorController(ID3D12DevicePtr device)
    : device(device),
      gpuCbvSrcUavHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE),
      gpuSamplerHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1024, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) {}

DescriptorAllocation DescriptorController::allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorsCount) {
    std::lock_guard<std::mutex> lock{allocateCpuLock};

    HeapVector &heapsForGivenType = this->cpuHeaps[type];
    std::unique_ptr<DescriptorAllocation> allocation = nullptr;

    // Try allocating in existing heaps
    for (auto &heap : heapsForGivenType) {
        allocation = heap.allocate(descriptorsCount);
        if (allocation != nullptr) {
            break;
        }
    }

    // If there was no space in existing heaps, create a new one
    if (allocation == nullptr) {
        constexpr static UINT baseHeapSize = 1024u; // TODO maybe allocate heaps of increasing size? Or make some heaps smaller (e.g. DSV)
        const auto heapSize = std::max(baseHeapSize, descriptorsCount);
        heapsForGivenType.emplace_back(device, type, heapSize, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        allocation = heapsForGivenType.back().allocate(descriptorsCount);
    }

    return std::move(*allocation.get());
}

DescriptorAllocation DescriptorController::allocateGpu(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorsCount) {
    std::lock_guard<std::mutex> lock{allocateGpuLock};

    DescriptorHeap &heap = getGpuHeap(type);
    auto allocation = heap.allocate(descriptorsCount);
    assert(allocation != nullptr);
    return std::move(*allocation.get());
}

DescriptorHeap &DescriptorController::getGpuHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) {
    switch (type) {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        return gpuCbvSrcUavHeap;
    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
        return gpuSamplerHeap;
    default:
        UNREACHABLE_CODE();
    }
}
