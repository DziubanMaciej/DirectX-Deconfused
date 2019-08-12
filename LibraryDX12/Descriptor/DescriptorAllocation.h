#pragma once

#include "Descriptor/CpuDescriptorHeap.h"

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"

/// \brief A given range of descriptors allocated
///
/// Objects of this class should not be created directly. CpuDescriptorHeap class should be used instead.
/// This class automatically manages deallocation in the heap object.
class DescriptorAllocation : DXD::NonCopyable {
public:
    /// CPU only descriptor handle. Used for staging descriptors
    DescriptorAllocation(CpuDescriptorHeap &heap, CpuDescriptorHeap::FreeListOffset offsetInHeap,
                            CpuDescriptorHeap::FreeListSize handlesCount, D3D12_CPU_DESCRIPTOR_HANDLE heapBaseHandle, UINT descriptorIncrementSize);

    /// GPU visible descriptor handle.
    DescriptorAllocation(CpuDescriptorHeap &heap, CpuDescriptorHeap::FreeListOffset offsetInHeap,
                            CpuDescriptorHeap::FreeListSize handlesCount, D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapBaseHandle,
                            D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapBaseHandle, UINT descriptorIncrementSize);

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
    CpuDescriptorHeap *heap;
    CpuDescriptorHeap::FreeListOffset offsetInHeap;
    CpuDescriptorHeap::FreeListSize handlesCount;
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    bool isGpuVisible;
    UINT descriptorIncrementSize;
};
