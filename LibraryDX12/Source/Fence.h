#pragma once

#include "DXD/NonCopyableAndMovable.h"
#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <stdint.h>

class Event;

class Fence : DXD::NonCopyable {
public:
    explicit Fence(ID3D12DevicePtr device);
    Fence(Fence &&other);
    Fence &operator=(Fence &&other);

    void wait(UINT64 fenceValue);
    void wait(UINT64 fenceValue, Event &fenceEvent);
    uint64_t signal(ID3D12CommandQueuePtr commandQueue);

    bool isComplete(UINT64 fenceValue) const;
    bool getFenceValue() const;

private:
    ID3D12FencePtr fence;
    uint64_t fenceValue = 0ull;
};
