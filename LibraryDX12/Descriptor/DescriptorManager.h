#pragma once

#include "Descriptor/DescriptorHeap.h"

#include <array>
#include <vector>

class DescriptorManager : DXD::NonCopyableAndMovable {
public:
    DescriptorManager(ID3D12DevicePtr device);
    DescriptorAllocation allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorsCount);
    DescriptorAllocation allocateGpu(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorsCount);

private:
    DescriptorHeap &getGpuHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);

    using HeapVector = std::vector<DescriptorHeap>;
    HeapVector cpuHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};
    DescriptorHeap gpuCbvSrcUavHeap;
    DescriptorHeap gpuSamplerHeap;
    ID3D12DevicePtr device;
};
