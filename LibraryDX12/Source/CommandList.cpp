#include "CommandList.h"
#include "Source/CommandAllocatorManager.h"
#include "Utility/ThrowIfFailed.h"

CommandList::CommandList(CommandAllocatorManager &commandAllocatorManager, ID3D12PipelineState *initialPipelineState)
    : commandAllocatorManager(commandAllocatorManager),
      commandAllocator(commandAllocatorManager.getCommandAllocator()),
      commandList(createCommandList(commandAllocatorManager, commandAllocator, initialPipelineState)) {}

CommandList::~CommandList() {
    commandAllocatorManager.registerAllocatorAndList(commandAllocator, commandList, fenceValue);
}

ID3D12GraphicsCommandListPtr CommandList::createCommandList(CommandAllocatorManager &commandAllocatorManager, ID3D12CommandAllocatorPtr commandAllocator, ID3D12PipelineState *initialPipelineState) {
    // First try to reuse existing command list
    ID3D12GraphicsCommandListPtr commandList = commandAllocatorManager.getCommandList(commandAllocator, initialPipelineState);
    if (commandList != nullptr) {
        return commandList;
    }

    // If there are no lists to reuse, allocate new
    ID3D12DevicePtr device = commandAllocatorManager.getDevice();
    D3D12_COMMAND_LIST_TYPE type = commandAllocatorManager.getType();
    throwIfFailed(device->CreateCommandList(0, type, commandAllocator.Get(), initialPipelineState, IID_PPV_ARGS(&commandList)));
    return commandList;
}
