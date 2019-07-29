#pragma once

#include "Descriptor/CpuDescriptorHeap.h"

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"

/// \brief A given number of CPU visible descriptors allocated
///
/// Objects of this class should not be created directly. CpuDescriptorHeap class should be used instead.
/// This class automatically manages deallocation in the heap object.
class CpuDescriptorAllocation : DXD::NonCopyable {
public:
    CpuDescriptorAllocation(CpuDescriptorHeap &heap, CpuDescriptorHeap::FreeListOffset offsetInHeap,
                            CpuDescriptorHeap::FreeListSize handlesCount, D3D12_CPU_DESCRIPTOR_HANDLE heapBaseHandle, UINT descriptorIncrementSize);
    CpuDescriptorAllocation(CpuDescriptorAllocation &&other);
    CpuDescriptorAllocation &operator=(CpuDescriptorAllocation &&other);
    ~CpuDescriptorAllocation();

    auto getOffsetInHeap() const { return offsetInHeap; };
    auto getHandlesCount() const { return handlesCount; }
    CD3DX12_CPU_DESCRIPTOR_HANDLE getCpuHandle() const;
    CD3DX12_CPU_DESCRIPTOR_HANDLE getCpuHandle(UINT offset) const;

private:
    CpuDescriptorHeap *heap;
    CpuDescriptorHeap::FreeListOffset offsetInHeap;
    CpuDescriptorHeap::FreeListSize handlesCount;
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    UINT descriptorIncrementSize;
};
