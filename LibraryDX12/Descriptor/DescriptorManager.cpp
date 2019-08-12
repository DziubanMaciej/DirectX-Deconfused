#include "DescriptorManager.h"

#include "Descriptor/DescriptorAllocation.h"

DescriptorManager::DescriptorManager(ID3D12DevicePtr device)
    : device(device) {}

DescriptorAllocation DescriptorManager::allocate(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorsCount) {
    HeapVector &heapsForGivenType = this->heaps[type];
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
        heapsForGivenType.emplace_back(device, type, heapSize);
        allocation = heapsForGivenType.back().allocate(descriptorsCount);
    }

    return std::move(*allocation.get());
}
