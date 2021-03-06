#include "CommandAllocatorController.h"

#include "Synchronization/Fence.h"
#include "Utility/ThrowIfFailed.h"

#include <cassert>

CommandAllocatorController::CommandAllocatorController(ID3D12DevicePtr device, Fence &fence, D3D12_COMMAND_LIST_TYPE type)
    : device(device), fence(fence), type(type) {}

ID3D12CommandAllocatorPtr CommandAllocatorController::retrieveCommandAllocator() {
    std::lock_guard<std::mutex> lock{this->commandAllocatorMutex};
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

ID3D12GraphicsCommandListPtr CommandAllocatorController::retrieveCommandList(ID3D12CommandAllocatorPtr commandAllocator, ID3D12PipelineState *initialPipelineState) {
    std::lock_guard<std::mutex> lock{this->commandListMutex};
    if (commandLists.empty()) {
        // Allocate new command list
        ID3D12GraphicsCommandListPtr commandList;
        throwIfFailed(device->CreateCommandList(0, type, commandAllocator.Get(), initialPipelineState, IID_PPV_ARGS(&commandList)));
        return commandList;
    }

    // We can reuse any command list
    ID3D12GraphicsCommandListPtr commandList = commandLists.front();
    commandLists.pop_front();
    commandList->Reset(commandAllocator.Get(), initialPipelineState);
    return commandList;
}

ID3D12CommandAllocatorPtr CommandAllocatorController::createCommandAllocator() {
    ID3D12CommandAllocatorPtr commandAllocator;
    throwIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));
    return commandAllocator;
}

void CommandAllocatorController::registerAllocatorAndList(ID3D12CommandAllocatorPtr commandAllocator, ID3D12GraphicsCommandListPtr commandList, uint64_t fence) {
    std::lock_guard<std::mutex> allocatorLock{this->commandAllocatorMutex};
    std::lock_guard<std::mutex> listLock{this->commandListMutex};

    commandAllocators.emplace_back(CommandAllocatorEntry{commandAllocator, fence});
    commandLists.push_back(commandList);
#if defined(_DEBUG)
    validateCommandAllocators();
    validateCommandLists();
#endif
}

void CommandAllocatorController::validateCommandAllocators() {
    for (auto i = 1u; i < commandAllocators.size(); i++) {
        const bool nondecreasing = commandAllocators[i - 1].lastFence <= commandAllocators[i].lastFence;
        assert(nondecreasing);
    }
}

void CommandAllocatorController::validateCommandLists() {
    for (const auto &commandList : commandLists) {
        // TODO it does not work but it should
        //const auto result = commandList->Close();
        //assert(result == E_FAIL);
    }
}
