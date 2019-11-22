#pragma once

#include "Synchronization/KernelEvent.h"

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/d3d12.h>
#include <stdint.h>

class Fence : DXD::NonCopyable {
public:
    explicit Fence(ID3D12DevicePtr device);
    Fence(Fence &&other) = default;
    Fence &operator=(Fence &&other) = default;

    void waitOnCpu(UINT64 fenceValue) const;
    uint64_t signal(ID3D12CommandQueuePtr commandQueue);

    bool isComplete(UINT64 fenceValue) const;
    uint64_t getLastSignalledFenceValue() const;
    uint64_t getCompletedFenceValue() const;

    auto getFence() const { return fence; }

private:
    ID3D12FencePtr fence;
    KernelEvent event;
    uint64_t lastSignalledFence = 0ull;
};
