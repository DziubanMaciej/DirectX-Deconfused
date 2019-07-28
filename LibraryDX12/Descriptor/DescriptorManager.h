#pragma once

#include "Descriptor/CpuDescriptorHeap.h"

#include <array>
#include <vector>

class DescriptorManager : DXD::NonCopyableAndMovable {
public:
    DescriptorManager(ID3D12DevicePtr device);
    CpuDescriptorAllocation allocate(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorsCount);

private:
    using HeapVector = std::vector<CpuDescriptorHeap>;
    HeapVector heaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    ID3D12DevicePtr device;
};
