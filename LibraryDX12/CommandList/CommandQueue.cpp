#include "CommandQueue.h"

#include "CommandList/CommandList.h"
#include "Utility/ThrowIfFailed.h"

// ------------------------------------------------------------------------------ Creating

CommandQueue::CommandQueue(ID3D12DevicePtr device, D3D12_COMMAND_LIST_TYPE type)
    : device(device),
      commandQueue(createCommandQueue(device, type)),
      type(type),
      fence(device),
      commandAllocatorController(device, fence, type) {
}

CommandQueue::~CommandQueue() {
    flush(true);
}

ID3D12CommandQueuePtr CommandQueue::createCommandQueue(ID3D12DevicePtr &device, D3D12_COMMAND_LIST_TYPE type) {
    ID3D12CommandQueuePtr commandQueue;

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    throwIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue)));

    return commandQueue;
}

// ------------------------------------------------------------------------------ Execute

uint64_t CommandQueue::signalOnGpu()
{
    return this->fence.signal(commandQueue);
}

bool CommandQueue::isFenceComplete(uint64_t fenceValue) const {
    return this->fence.isComplete(fenceValue);
}

void CommandQueue::waitOnCpu(uint64_t fenceValue) const {
    return this->fence.waitOnCpu(fenceValue);
}

void CommandQueue::waitOnGpu(const CommandQueue &queueToWaitFor, uint64_t fenceValue) {
    this->commandQueue->Wait(queueToWaitFor.fence.getFence().Get(), fenceValue);
}

uint64_t CommandQueue::executeCommandListsAndSignal(std::vector<CommandList *> &commandLists) {
    std::unique_lock<std::mutex> lock = getLock(true);

    // Convert to vector of pointers to D3D command list objects
    std::vector<ID3D12CommandList *> commandListPtrs;
    for (auto &commandList : commandLists) {
        commandListPtrs.push_back(commandList->getCommandList().Get());
    }

    // Submit work to GPU and signal fence
    commandQueue->ExecuteCommandLists(static_cast<UINT>(commandListPtrs.size()), commandListPtrs.data());
    const uint64_t fenceValue = fence.signal(commandQueue);

    // Cleanup the CommandList objects, register child objects to GPU usage tracker
    for (auto &commandList : commandLists) {
        commandList->registerAllData(resourceUsageTracker, fenceValue);
    }

    // Return fence value used for waiting on this command list batch
    return fenceValue;
}

uint64_t CommandQueue::executeCommandListAndSignal(CommandList &commandList) {
    std::unique_lock<std::mutex> lock = getLock(true);

    // Submit work to GPU and signal fence
    ID3D12CommandList *commandListPtr = commandList.getCommandList().Get();
    commandQueue->ExecuteCommandLists(1u, &commandListPtr);
    const uint64_t fenceValue = fence.signal(commandQueue);

    // Cleanup the CommandList object, register child objects to GPU usage tracker
    commandList.registerAllData(resourceUsageTracker, fenceValue);

    // Return fence value used for waiting on this command list batch
    return fenceValue;
}

void CommandQueue::performResourcesDeletion(bool lockQueue) {
    std::unique_lock<std::mutex> lock = getLock(lockQueue);
    resourceUsageTracker.performDeletion(fence.getCompletedFenceValue());
}

// ------------------------------------------------------------------------------ Wait and signal

void CommandQueue::flush(bool lockQueue) {
    std::unique_lock<std::mutex> lock = getLock(lockQueue);
    const uint64_t newFenceValue = fence.signal(commandQueue);
    fence.waitOnCpu(newFenceValue);
}

std::unique_lock<std::mutex> CommandQueue::getLock(bool lockQueue) {
    std::unique_lock<std::mutex> lock{this->lock, std::defer_lock};
    if (lockQueue) {
        lock.lock();
    }
    return std::move(lock);
}
