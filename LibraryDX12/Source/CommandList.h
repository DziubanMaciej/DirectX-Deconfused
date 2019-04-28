#pragma once

#include "DXD/NonCopyableAndMovable.h"
#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <cstdint>

class CommandAllocatorManager;

class CommandList : DXD::NonCopyableAndMovable {
public:
    CommandList(CommandAllocatorManager &commandAllocatorManager, ID3D12PipelineState *initialPipelineState);
    ~CommandList();

    void setFenceValue(uint64_t fenceValue) { this->fenceValue = fenceValue; }

    auto getCommandList() { return commandList; }

private:
    ID3D12GraphicsCommandListPtr createCommandList(CommandAllocatorManager &commandAllocatorManager, ID3D12CommandAllocatorPtr commandAllocator, ID3D12PipelineState *initialPipelineState);

    CommandAllocatorManager &commandAllocatorManager;
    ID3D12CommandAllocatorPtr commandAllocator;
    ID3D12GraphicsCommandListPtr commandList;
    uint64_t fenceValue;
};
