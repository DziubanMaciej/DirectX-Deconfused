#include "CpuDescriptorHeap.h"

#include "Descriptor/CpuDescriptorAllocation.h"
#include "Utility/ThrowIfFailed.h"

CpuDescriptorHeap::CpuDescriptorHeap(ID3D12DevicePtr device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorsCount)
    : type(type),
      descriptorIncrementSize(device->GetDescriptorHandleIncrementSize(type)),
      heap(createDescriptorHeap(device, type, descriptorsCount)),
      heapStartHandle(heap->GetCPUDescriptorHandleForHeapStart()),
      totalFreeSpace(descriptorsCount) {
    freeList.emplace(0, descriptorsCount);
}

std::unique_ptr<CpuDescriptorAllocation> CpuDescriptorHeap::allocate(UINT descriptorsCount) {
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
            return std::make_unique<CpuDescriptorAllocation>(*this, offsetInHeap, descriptorsCount, this->heapStartHandle, this->descriptorIncrementSize);
        }
    }

    // Failure to allocate
    return nullptr;
}

void CpuDescriptorHeap::deallocate(const CpuDescriptorAllocation &allocation) {
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
            freeList.erase(right);
            freeList.emplace(offset, size + right->second); // TODO <-- is it valid?
        } else {
            // cannot merge with anything
            freeList.emplace(offset, size);
        }
    }

    totalFreeSpace += size;
}

ID3D12DescriptorHeapPtr CpuDescriptorHeap::createDescriptorHeap(ID3D12DevicePtr device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorsCount) {
    D3D12_DESCRIPTOR_HEAP_DESC heapDescription = {};
    heapDescription.Type = type;
    heapDescription.NumDescriptors = descriptorsCount;
    heapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // non shader visible
    heapDescription.NodeMask = 1;

    ID3D12DescriptorHeapPtr descriptorHeap;
    throwIfFailed(device->CreateDescriptorHeap(&heapDescription, IID_PPV_ARGS(&descriptorHeap)));
    return descriptorHeap;
}

CpuDescriptorHeap::FreeList::iterator CpuDescriptorHeap::findFreeRangeAfter(FreeListOffset offset) {
    return freeList.upper_bound(offset);
}

CpuDescriptorHeap::FreeList::iterator CpuDescriptorHeap::findPreviousRange(FreeList::iterator elementAfter) {
    if (elementAfter == freeList.begin()) {
        return freeList.end();
    } else {
        return --elementAfter;
    }
}

bool CpuDescriptorHeap::areFreeRangesAdjacent(FreeList::iterator left, FreeList::iterator right) {
    return right != freeList.end() && areFreeRangesAdjacent(left, right->first);
}

bool CpuDescriptorHeap::areFreeRangesAdjacent(FreeList::iterator left, FreeListOffset rightOffset) {
    return left != freeList.end() && left->first + left->second == rightOffset;
}

bool CpuDescriptorHeap::areFreeRangesAdjacent(FreeListOffset leftOffset, FreeListSize leftSize, FreeList::iterator right) {
    return right != freeList.end() && leftOffset + leftSize == right->first;
}
