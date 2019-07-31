#pragma once

#include "CommandList/CommandAllocatorManager.h"
#include "Descriptor/GpuDescriptorHeapController.h"
#include "PipelineState/PipelineStateController.h"

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <cstdint>
#include <set>
#include <vector>

class VertexBuffer;
class IndexBuffer;
class CpuDescriptorAllocation;

/// Class encapsulating DX12 command list
class CommandList : DXD::NonCopyableAndMovable {
public:
    /// Retrieves command list and allocator from the commandAllocatorManager's pool
    CommandList(CommandAllocatorManager &commandAllocatorManager, ID3D12PipelineState *initialPipelineState);
    /// Registers the command list and allocator back to the commandAllocatorManager for later reuse
    ~CommandList();

    void transitionBarrierSingle(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);

    void clearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const FLOAT colorRGBA[4]);
    void clearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth, UINT8 stencil);

    void setGraphicsRootSignature(ID3D12RootSignaturePtr rootSignature);
    void setPipelineState(ID3D12PipelineStatePtr pipelineState);
    void setPipelineStateAndGraphicsRootSignature(PipelineStateController &pipelineStateController, PipelineStateController::Identifier identifier);

    void setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeapPtr descriptorHeap);

    void setCbvSrvUavDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, D3D12_CPU_DESCRIPTOR_HANDLE firstDescriptor, UINT descriptorCount);
    void setCbvSrvUavDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, CpuDescriptorAllocation &cpuDescriptorAllocation);
    void setCbvSrvUavDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, CpuDescriptorAllocation &cpuDescriptorAllocation, UINT descriptorCount);

    void IASetVertexBuffers(UINT startSlot, UINT numBuffers, const VertexBuffer *vertexBuffers);
    void IASetVertexBuffer(UINT slot, const VertexBuffer &vertexBuffer);
    void IASetVertexBuffer(const VertexBuffer &vertexBuffer);
    void IASetIndexBuffer(const IndexBuffer &indexBuffer);
    void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);
    void IASetPrimitiveTopologyTriangleList();

    void RSSetViewports(UINT numViewports, const D3D12_VIEWPORT *viewports);
    void RSSetViewport(const D3D12_VIEWPORT &viewport);
    void RSSetScissorRects(UINT numRects, const D3D12_RECT *rects);
    void RSSetScissorRect(const D3D12_RECT &rect);

    void OMSetRenderTargets(UINT renderTargetsCount, const D3D12_CPU_DESCRIPTOR_HANDLE *renderTargetDescriptors,
                            const ID3D12ResourcePtr *renderTargets, BOOL singleHandleToDescriptorRange,
                            const D3D12_CPU_DESCRIPTOR_HANDLE &depthStencilDescriptor, const ID3D12ResourcePtr &depthStencilBuffer);
    void OMSetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE &renderTargetDescriptor, const ID3D12ResourcePtr &renderTarget,
                           const D3D12_CPU_DESCRIPTOR_HANDLE &depthStencilDescriptor, const ID3D12ResourcePtr &depthStencilBuffer);

    template <typename ConstantType>
    void setGraphicsRoot32BitConstant(UINT rootParameterIndex, const ConstantType &constant);

    void drawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation);
    void drawIndexed(UINT indexCount, UINT startIndexLocation = 0u, INT baseVertexLocation = 0u, UINT startInstanceLocation = 0u);
    void drawInstanced(UINT vertexCountPerInstance, UINT instanceCount, INT baseVertexLocation, UINT startInstanceLocation);

    void close();

    void addUsedResource(const ID3D12DescriptorHeapPtr &heap);
    void addUsedResource(const ID3D12ResourcePtr &resource);
    void addUsedResources(const ID3D12ResourcePtr *resources, UINT resourcesCount);
    void setFenceValue(uint64_t fenceValue) { this->fenceValue = fenceValue; }

    auto getCommandList() { return commandList; }
    auto &getUsedResources() { return usedResources; }
    auto getDevice() const { return commandAllocatorManager.getDevice(); }

private:
    void commitDescriptors();

    // Base CommandList data
    CommandAllocatorManager &commandAllocatorManager;
    ID3D12CommandAllocatorPtr commandAllocator;
    ID3D12GraphicsCommandListPtr commandList;
    uint64_t fenceValue;

    // Descriptor controllers
    GpuDescriptorHeapController gpuDescriptorHeapControllerSampler;
    GpuDescriptorHeapController gpuDescriptorHeapControllerCbvSrvUav;

    // Tracked resources
    ID3D12DescriptorHeapPtr descriptorHeapCbvSrvUav = {};
    ID3D12DescriptorHeapPtr descriptorHeapSampler = {};
    std::set<ID3D12PageablePtr> usedResources = {};
};

template <typename ConstantType>
inline void CommandList::setGraphicsRoot32BitConstant(UINT rootParameterIndex, const ConstantType &constant) {
    static_assert(sizeof(ConstantType) % 4 == 0, "Not a dword aligned type");
    constexpr static auto dwordCount = sizeof(ConstantType) / 4;
    commandList->SetGraphicsRoot32BitConstants(rootParameterIndex, dwordCount, &constant, 0);
}
