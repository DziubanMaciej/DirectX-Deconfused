#include "CommandQueue.h"

#include "Utility/ThrowIfFailed.h"
#include <cassert>

// ------------------------------------------------------------------------------ Creating

CommandQueue::CommandQueue(ID3D12DevicePtr device, D3D12_COMMAND_LIST_TYPE type)
    : device(device),
      commandQueue(createCommandQueue(device, type)),
      type(type),
      fence(device),
      commandAllocatorManager(device, fence, type) {
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

uint64_t CommandQueue::executeCommandListsAndSignal(std::vector<ID3D12GraphicsCommandListPtr> &commandListComPtrs) {
    std::vector<ID3D12CommandList *> commandListPtrs;
    for (auto &commandListComPtr : commandListComPtrs) {
        commandListPtrs.push_back(commandListComPtr.Get());
    }
    return executeCommandListsAndSignal(commandListPtrs);
}

// ------------------------------------------------------------------------------ Wait and signal

void CommandQueue::flush(Event &fenceEvent) {
    const uint64_t newFenceValue = fence.signal(commandQueue);
    fence.wait(newFenceValue, fenceEvent);
}

void CommandQueue::wait(Event &fenceEvent) {
    fence.wait(fence.getFenceValue(), fenceEvent);
}

// ------------------------------------------------------------------------------ Command Allocator manager

CommandAllocatorManager::CommandAllocatorManager(ID3D12DevicePtr device, Fence &fence, D3D12_COMMAND_LIST_TYPE type)
    : device(device), fence(fence), type(type) {}

ID3D12CommandAllocatorPtr CommandAllocatorManager::getCommandAllocator() {
    if (!commandAllocators.empty() && fence.isComplete(commandAllocators.front().lastFence)) {
        // First allocator is done, we can reuse it
        ID3D12CommandAllocatorPtr result = commandAllocators.front().commandAllocator;
        commandAllocators.pop_front();
        throwIfFailed(result->Reset());
        return result;
    } else {
        // Cannot reuse any allocator, return new
        return createCommandAllocator();
    }
}

ID3D12GraphicsCommandListPtr CommandAllocatorManager::getCommandList(ID3D12CommandAllocatorPtr commandAllocator, ID3D12PipelineState *initialPipelineState) {
    if (!commandLists.empty()) {
        // We can reuse any command list
        ID3D12GraphicsCommandListPtr commandList = commandLists.front();
        commandLists.pop_front();
        commandList->Reset(commandAllocator.Get(), initialPipelineState);
        return commandList;
    } else {
        // No command lists registered, create new
        return createCommandList(commandAllocator, initialPipelineState);
    }
}

ID3D12CommandAllocatorPtr CommandAllocatorManager::createCommandAllocator() {
    ID3D12CommandAllocatorPtr commandAllocator;
    throwIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));
    return commandAllocator;
}

ID3D12GraphicsCommandListPtr CommandAllocatorManager::createCommandList(ID3D12CommandAllocatorPtr commandAllocator, ID3D12PipelineState *initialPipelineState) {
    ID3D12GraphicsCommandListPtr commandList;
    throwIfFailed(device->CreateCommandList(0, type, commandAllocator.Get(), initialPipelineState, IID_PPV_ARGS(&commandList)));
    return commandList;
}

void CommandAllocatorManager::registerAllocatorAndList(ID3D12CommandAllocatorPtr commandAllocator, ID3D12GraphicsCommandListPtr commandList, uint64_t fence) {
    commandAllocators.emplace_back(CommandAllocatorEntry{commandAllocator, fence});
    commandLists.push_back(commandList);
#if defined(_DEBUG)
    validateCommandAllocators();
    validateCommandLists();
#endif
}

void CommandAllocatorManager::validateCommandAllocators() {
    for (auto i = 1u; i < commandAllocators.size(); i++) {
        const bool nondecreasing = commandAllocators[i - 1].lastFence <= commandAllocators[i].lastFence;
        assert(nondecreasing);
    }
}

void CommandAllocatorManager::validateCommandLists() {
    for (const auto &commandList : commandLists) {
        // TODO it does not work but it should
        //const auto result = commandList->Close();
        //assert(result == E_FAIL);
    }
}
