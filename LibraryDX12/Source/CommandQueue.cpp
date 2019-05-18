#include "CommandQueue.h"
#include "Source/CommandList.h"
#include "Utility/ThrowIfFailed.h"

// ------------------------------------------------------------------------------ Creating

CommandQueue::CommandQueue(ID3D12DevicePtr device, D3D12_COMMAND_LIST_TYPE type)
    : device(device),
      commandQueue(createCommandQueue(device, type)),
      type(type),
      fence(device),
      commandAllocatorManager(device, fence, type) {
}

CommandQueue::~CommandQueue() {
    flush();
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

uint64_t CommandQueue::executeCommandListsAndSignal(std::vector<ID3D12CommandList *> &commandListPtrs) {
    commandQueue->ExecuteCommandLists(static_cast<UINT>(commandListPtrs.size()), commandListPtrs.data());
    return fence.signal(commandQueue);
}

uint64_t CommandQueue::executeCommandListsAndSignal(std::vector<CommandList *> &commandLists) {
    std::vector<ID3D12CommandList *> commandListPtrs;
    for (auto &commandList : commandLists) {
        commandListPtrs.push_back(commandList->getCommandList().Get());
    }
    const uint64_t fenceValue = executeCommandListsAndSignal(commandListPtrs);

    for (auto &commandList : commandLists) {
        commandList->setFenceValue(fenceValue);
        resourceUsageTracker.registerUsage(commandList->getUsedResources(), fenceValue);
    }

    return fenceValue;
}

void CommandQueue::performResourcesDeletion() {
    resourceUsageTracker.performDeletion(fence.getFenceValue());
}

// ------------------------------------------------------------------------------ Wait and signal

void CommandQueue::flush() {
    const uint64_t newFenceValue = fence.signal(commandQueue);
    fence.waitOnCpu(newFenceValue);
}
