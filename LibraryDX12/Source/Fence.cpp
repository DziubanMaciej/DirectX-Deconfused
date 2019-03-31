#include "Fence.h"

#include "Source/Event.h"
#include "Utility/ThrowIfFailed.h"

Fence::Fence(ID3D12DevicePtr device) {
    throwIfFailed(device->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
}

Fence::Fence(Fence &&other) {
    *this = std::move(other);
}

Fence &Fence::operator=(Fence &&other) {
    this->fence = std::move(other.fence);
    return *this;
}

void Fence::wait(UINT64 fenceValue) {
    Event event;
    wait(fenceValue, event);
}

void Fence::wait(UINT64 fenceValue, Event &fenceEvent) {
    if (isComplete(fenceValue)) {
        return;
    }
    throwIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent.getHandle()));
    fenceEvent.wait(INFINITE);
}

uint64_t Fence::signal(ID3D12CommandQueuePtr commandQueue) {
    // Updates fenceValue to value being signaled
    fenceValue++;
    throwIfFailed(commandQueue->Signal(fence.Get(), fenceValue));
    return fenceValue;
}

bool Fence::isComplete(UINT64 fenceValue) const {
    return fence->GetCompletedValue() >= fenceValue;
}

bool Fence::getFenceValue() const
{
    return fenceValue;
}
