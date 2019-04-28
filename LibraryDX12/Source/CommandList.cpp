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

void CommandList::transitionBarrierSingle(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter) {
    const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), stateBefore, stateAfter);
    commandList->ResourceBarrier(1, &barrier);
}

void CommandList::transitionBarrierMultiple(const std::vector<D3D12_RESOURCE_BARRIER> &barriers) {
    if (barriers.size() != 0) {
        commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
    }
}

void CommandList::clearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const FLOAT colorRGBA[4]) {
    commandList->ClearRenderTargetView(renderTargetView, colorRGBA, 0, nullptr);
}

void CommandList::clearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth, UINT8 stencil) {
    commandList->ClearDepthStencilView(depthStencilView, clearFlags, depth, stencil, 0, nullptr);
}

void CommandList::close() {
    throwIfFailed(commandList->Close());
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
