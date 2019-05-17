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

    void setPipelineState(ID3D12PipelineStatePtr pipelineState);
    void setGraphicsRootSignature(ID3D12RootSignaturePtr rootSignature);

    void IASetVertexBuffers(UINT startSlot, UINT numBuffers, const D3D12_VERTEX_BUFFER_VIEW *views);
    void IASetVertexBuffer(UINT slot, const D3D12_VERTEX_BUFFER_VIEW &view);
    void IASetVertexBuffer(const D3D12_VERTEX_BUFFER_VIEW &view);
    void IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW &view);
    void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);
    void IASetPrimitiveTopologyTriangleList();

    void RSSetViewports(UINT numViewports, const D3D12_VIEWPORT *viewports);
    void RSSetViewport(const D3D12_VIEWPORT &viewport);
    void RSSetScissorRects(UINT numRects, const D3D12_RECT *rects);
    void RSSetScissorRect(const D3D12_RECT &rect);

    void OMSetRenderTargets(UINT rtDescriptorsCount, const D3D12_CPU_DESCRIPTOR_HANDLE *rtDescriptors, BOOL singleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE &depthStencilDescriptor);
    void OMSetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE &rtDescriptor, const D3D12_CPU_DESCRIPTOR_HANDLE &depthStencilDescriptor);

    template <typename ConstantType>
    void setGraphicsRoot32BitConstant(UINT rootParameterIndex, const ConstantType &constant);

    void drawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation);
    void drawIndexed(UINT indexCount, UINT startIndexLocation = 0u, INT baseVertexLocation = 0u, UINT startInstanceLocation = 0u);

    void close();

    void setFenceValue(uint64_t fenceValue) { this->fenceValue = fenceValue; }

    auto getCommandList() { return commandList; }

private:
    CommandAllocatorManager &commandAllocatorManager;
    ID3D12CommandAllocatorPtr commandAllocator;
    ID3D12GraphicsCommandListPtr commandList;
    uint64_t fenceValue;
};

template <typename ConstantType>
inline void CommandList::setGraphicsRoot32BitConstant(UINT rootParameterIndex, const ConstantType &constant) {
    static_assert(sizeof(ConstantType) % 4 == 0, "Not a dword aligned type");
    constexpr static auto dwordCount = sizeof(ConstantType) / 4;
    commandList->SetGraphicsRoot32BitConstants(rootParameterIndex, dwordCount, &constant, 0);
}
