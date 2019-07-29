#pragma once

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <map>
#include <memory>

class CpuDescriptorAllocation;

/// \brief Manages a single descriptor heap of given type and size
///
/// Provides simplistic interface for allocating given number of descriptors in CPU visible descriptor heap.
/// These descriptors cannot be bound to command list and have to be later copied to the GPU visible descriptor
/// heap at recording time.
///
/// Class manages free list to keep track of free space inside the heap. Allocation means finding space in
/// free list, delete it and returning CpuDescriptorAllocation object, which encapsulates cpu descriptor and
/// allocation size in descriptors. Allocation can fail if there's not enough space. Proper deallocation is
/// handled automatically upon destruction of CpuDescriptorAllocation object and it consists of returning space
/// taken by CpuDescriptorAllocation to the free list. If space is adjacent to already existing, block are merged.
class CpuDescriptorHeap : DXD::NonCopyableAndMovable {
public:
    CpuDescriptorHeap(ID3D12DevicePtr device, D3D12_DESCRIPTOR_HEAP_TYPE type, int descriptorsCount);

    /// Finds free space using free list, marks the space as not free and returns allocation object
    /// \param descriptorsCount quantity of descriptors to allocate. Free list is searched for contiguous block
    /// \return valid allocation object or nullptr
    std::unique_ptr<CpuDescriptorAllocation> allocate(int descriptorsCount);

private:
    using FreeListOffset = int;
    using FreeListSize = int;
    using FreeList = std::map<FreeListOffset, FreeListSize>;

    /// Should only be called by CpuDescriptorAllocation. Descriptor block is merged with existing ones if possible
    /// \param allocation allocation object to free
    void deallocate(const CpuDescriptorAllocation &allocation);
    friend CpuDescriptorAllocation; // for deallocate()

    static ID3D12DescriptorHeapPtr createDescriptorHeap(ID3D12DevicePtr device, D3D12_DESCRIPTOR_HEAP_TYPE type, int descriptorsCount);
    FreeList::iterator findFreeRangeAfter(FreeListOffset offset);
    FreeList::iterator findPreviousRange(FreeList::iterator elementAfter);
    bool areFreeRangesAdjacent(FreeList::iterator left, FreeList::iterator right);
    bool areFreeRangesAdjacent(FreeList::iterator left, FreeListOffset rightOffset);
    bool areFreeRangesAdjacent(FreeListOffset leftOffset, FreeListSize leftSize, FreeList::iterator right);

    const D3D12_DESCRIPTOR_HEAP_TYPE type;
    const UINT descriptorIncrementSize;
    const ID3D12DescriptorHeapPtr heap;
    const D3D12_CPU_DESCRIPTOR_HANDLE heapStartHandle;
    FreeList freeList;
    int totalFreeSpace;
};
