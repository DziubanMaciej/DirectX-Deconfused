#pragma once

#include "Descriptor/DescriptorHeap.h"

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"

/// \brief A given range of descriptors allocated
///
/// Objects of this class should not be created directly. DescriptorHeap class should be used instead.
/// This class automatically manages deallocation in the heap object.
class DescriptorAllocation : DXD::NonCopyable {
public:
    /// CPU only descriptor handle. Used for staging descriptors
    DescriptorAllocation(DescriptorHeap &heap, DescriptorHeap::FreeListOffset offsetInHeap,
                         DescriptorHeap::FreeListSize handlesCount, D3D12_CPU_DESCRIPTOR_HANDLE heapBaseHandle, UINT descriptorIncrementSize);

    /// GPU visible descriptor handle.
    DescriptorAllocation(DescriptorHeap &heap, DescriptorHeap::FreeListOffset offsetInHeap,
                         DescriptorHeap::FreeListSize handlesCount, D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapBaseHandle,
                         D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapBaseHandle, UINT descriptorIncrementSize);

    bool operator<(const DescriptorAllocation &allocation) const {
        return cpuHandle.ptr < allocation.cpuHandle.ptr;
    }

    DescriptorAllocation(const DescriptorAllocation &other) = delete;
    DescriptorAllocation &operator=(const DescriptorAllocation &other) = delete;

    DescriptorAllocation(DescriptorAllocation &&other);
    DescriptorAllocation &operator=(DescriptorAllocation &&other);
    ~DescriptorAllocation();

    auto getOffsetInHeap() const { return offsetInHeap; };
    auto getHandlesCount() const { return handlesCount; }
    CD3DX12_CPU_DESCRIPTOR_HANDLE getCpuHandle() const;
    CD3DX12_CPU_DESCRIPTOR_HANDLE getCpuHandle(UINT offset) const;
    CD3DX12_GPU_DESCRIPTOR_HANDLE getGpuHandle() const;
    CD3DX12_GPU_DESCRIPTOR_HANDLE getGpuHandle(UINT offset) const;

private:
    DescriptorHeap *heap;
    DescriptorHeap::FreeListOffset offsetInHeap;
    DescriptorHeap::FreeListSize handlesCount;
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    bool isGpuVisible;
    UINT descriptorIncrementSize;
};
