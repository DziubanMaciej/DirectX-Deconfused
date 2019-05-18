#include "Fence.h"
#include "Utility/ThrowIfFailed.h"

Fence::Fence(ID3D12DevicePtr device) {
    throwIfFailed(device->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
}

void Fence::waitOnCpu(UINT64 fenceValue) {
    if (isComplete(fenceValue)) {
        return;
    }
    throwIfFailed(fence->SetEventOnCompletion(fenceValue, event.getHandle()));
    event.wait();
}

void Fence::waitOnGpu(ID3D12CommandQueuePtr commandQueue, UINT64 fenceValue) {
    commandQueue->Wait(fence.Get(), fenceValue);
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

bool Fence::getFenceValue() const {
    return fenceValue;
}
