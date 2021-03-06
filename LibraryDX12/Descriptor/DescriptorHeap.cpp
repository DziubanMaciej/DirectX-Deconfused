#include "DescriptorHeap.h"

#include "Descriptor/DescriptorAllocation.h"
#include "Utility/ThrowIfFailed.h"

DescriptorHeap::DescriptorHeap(ID3D12DevicePtr device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorsCount, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
    : type(type),
      descriptorIncrementSize(device->GetDescriptorHandleIncrementSize(type)),
      heap(createDescriptorHeap(device, type, descriptorsCount, flags)),
      gpuVisible(flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE),
      cpuHeapStartHandle(heap->GetCPUDescriptorHandleForHeapStart()),
      gpuHeapStartHandle(gpuVisible ? heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{}),
      totalFreeSpace(descriptorsCount) {
    freeList.emplace(0, descriptorsCount);
}

std::unique_ptr<DescriptorAllocation> DescriptorHeap::allocate(UINT descriptorsCount) {
    std::lock_guard<std::mutex> lock{this->lock};

    if (descriptorsCount > totalFreeSpace) {
        return false;
    }
    totalFreeSpace -= descriptorsCount;

    // Look for contiguous free region
    for (auto it = freeList.begin(); it != freeList.end(); it++) {
        const auto freeRangeOffset = it->first;
        const auto freeRangeSize = it->second;

        // If there's a contiguous block, shrink it and return the allocation
        if (freeRangeSize >= descriptorsCount) {
            if (freeRangeSize == descriptorsCount) {
                freeList.erase(it);
            } else {
                it->second -= descriptorsCount;
            }

            const auto offsetInHeap = freeRangeOffset + freeRangeSize - descriptorsCount;
            if (gpuVisible) {
                return std::make_unique<DescriptorAllocation>(*this, offsetInHeap, descriptorsCount, this->cpuHeapStartHandle,
                                                              this->gpuHeapStartHandle, this->descriptorIncrementSize);
            } else {
                return std::make_unique<DescriptorAllocation>(*this, offsetInHeap, descriptorsCount, this->cpuHeapStartHandle,
                                                              this->descriptorIncrementSize);
            }
        }
    }

    // Failure to allocate
    return nullptr;
}

void DescriptorHeap::deallocate(const DescriptorAllocation &allocation) {
    std::lock_guard<std::mutex> lock{this->lock};

    const auto offset = allocation.getOffsetInHeap();
    const auto size = allocation.getHandlesCount();
    auto right = findFreeRangeAfter(offset);
    auto left = findPreviousRange(right);

    if (areFreeRangesAdjacent(left, offset)) {
        // merge with left range
        left->second += size;

        if (areFreeRangesAdjacent(left, right)) {
            // merge with left and right range
            left->second += right->second;
            freeList.erase(right);
        }
    } else {
        // cannot merge with left range
        if (areFreeRangesAdjacent(offset, size, right)) {
            // cannot merge with left range, but can merge with right range
            const auto rightSize = right->second;
            freeList.erase(right);
            freeList.emplace(offset, size + rightSize);
        } else {
            // cannot merge with anything
            freeList.emplace(offset, size);
        }
    }

    totalFreeSpace += size;
}

ID3D12DescriptorHeapPtr DescriptorHeap::createDescriptorHeap(ID3D12DevicePtr device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorsCount, D3D12_DESCRIPTOR_HEAP_FLAGS flags) {
    D3D12_DESCRIPTOR_HEAP_DESC heapDescription = {};
    heapDescription.Type = type;
    heapDescription.NumDescriptors = descriptorsCount;
    heapDescription.Flags = flags;
    heapDescription.NodeMask = 1;

    ID3D12DescriptorHeapPtr descriptorHeap;
    throwIfFailed(device->CreateDescriptorHeap(&heapDescription, IID_PPV_ARGS(&descriptorHeap)));
    return descriptorHeap;
}

DescriptorHeap::FreeList::iterator DescriptorHeap::findFreeRangeAfter(FreeListOffset offset) {
    return freeList.upper_bound(offset);
}

DescriptorHeap::FreeList::iterator DescriptorHeap::findPreviousRange(FreeList::iterator elementAfter) {
    if (elementAfter == freeList.begin()) {
        return freeList.end();
    } else {
        return --elementAfter;
    }
}

bool DescriptorHeap::areFreeRangesAdjacent(FreeList::iterator left, FreeList::iterator right) {
    return right != freeList.end() && areFreeRangesAdjacent(left, right->first);
}

bool DescriptorHeap::areFreeRangesAdjacent(FreeList::iterator left, FreeListOffset rightOffset) {
    return left != freeList.end() && left->first + left->second == rightOffset;
}

bool DescriptorHeap::areFreeRangesAdjacent(FreeListOffset leftOffset, FreeListSize leftSize, FreeList::iterator right) {
    return right != freeList.end() && leftOffset + leftSize == right->first;
}
