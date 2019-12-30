#pragma once

#include "Descriptor/DescriptorHeap.h"

#include <array>
#include <vector>
#include <mutex>

class DescriptorController : DXD::NonCopyableAndMovable {
public:
    DescriptorController(ID3D12DevicePtr device);
    DescriptorAllocation allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorsCount);
    DescriptorAllocation allocateGpu(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptorsCount);

private:
    DescriptorHeap &getGpuHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);

    using HeapVector = std::vector<DescriptorHeap>;
    HeapVector cpuHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};
    DescriptorHeap gpuCbvSrcUavHeap;
    DescriptorHeap gpuSamplerHeap;
    ID3D12DevicePtr device;
    std::mutex allocateCpuLock = {};
    std::mutex allocateGpuLock = {};
};
