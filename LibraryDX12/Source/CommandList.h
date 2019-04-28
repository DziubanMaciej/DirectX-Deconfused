#pragma once

#include "DXD/NonCopyableAndMovable.h"
#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <cstdint>
#include <vector>

class CommandAllocatorManager;

/// Class encapsulating DX12 command list
class CommandList : DXD::NonCopyableAndMovable {
public:
    /// Retrieves command list and allocator from the commandAllocatorManager's pool
    CommandList(CommandAllocatorManager &commandAllocatorManager, ID3D12PipelineState *initialPipelineState);
    /// Registers the command list and allocator back to the commandAllocatorManager for later reuse
    ~CommandList();

    void transitionBarrierSingle(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
    void transitionBarrierMultiple(const std::vector<D3D12_RESOURCE_BARRIER> &barriers);

    void clearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const FLOAT colorRGBA[4]);
    void clearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth, UINT8 stencil);

    void close();

    void setFenceValue(uint64_t fenceValue) { this->fenceValue = fenceValue; }

    auto getCommandList() { return commandList; }

private:
    ID3D12GraphicsCommandListPtr createCommandList(CommandAllocatorManager &commandAllocatorManager, ID3D12CommandAllocatorPtr commandAllocator, ID3D12PipelineState *initialPipelineState);

    CommandAllocatorManager &commandAllocatorManager;
    ID3D12CommandAllocatorPtr commandAllocator;
    ID3D12GraphicsCommandListPtr commandList;
    uint64_t fenceValue;
};
