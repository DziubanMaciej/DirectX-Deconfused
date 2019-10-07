#include "Fence.h"
#include "Utility/ThrowIfFailed.h"

Fence::Fence(ID3D12DevicePtr device) {
    throwIfFailed(device->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
}

void Fence::waitOnCpu(UINT64 fenceValue) const {
    if (isComplete(fenceValue)) {
        return;
    }
    throwIfFailed(fence->SetEventOnCompletion(fenceValue, event.getHandle()));
    event.wait();
}

uint64_t Fence::signal(ID3D12CommandQueuePtr commandQueue) {
    // Updates fenceValue to value being signaled
    lastSignalledFence++;
    throwIfFailed(commandQueue->Signal(fence.Get(), lastSignalledFence));
    return lastSignalledFence;
}

bool Fence::isComplete(UINT64 fenceValue) const {
    return fence->GetCompletedValue() >= fenceValue;
}

uint64_t Fence::getLastSignalledFenceValue() const {
    return lastSignalledFence;
}

uint64_t Fence::getCompletedFenceValue() const {
    return fence->GetCompletedValue();
}
