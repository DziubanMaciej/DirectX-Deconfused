#pragma once

#include "CommandList/CommandAllocatorController.h"
#include "Descriptor/DescriptorController.h"
#include "Descriptor/GpuDescriptorHeapController.h"
#include "PipelineState/PipelineStateController.h"
#include "Resource/ResourceUsageTracker.h"

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <cstdint>
#include <set>
#include <vector>

class VertexBuffer;
class IndexBuffer;
class DescriptorAllocation;
class MeshImpl;
class Resource;

/// Class encapsulating DX12 command list
class CommandList : DXD::NonCopyableAndMovable {
public:
    /// Retrieves command list and allocator from the CommandAllocatorController's pool
    CommandList(DescriptorController &descriptorController, CommandAllocatorController &commandAllocatorController, ID3D12PipelineState *initialPipelineState);
    /// Registers the command list and allocator back to the commandAllocatorController for later reuse
    ~CommandList();

    void transitionBarrier(Resource &resource, D3D12_RESOURCE_STATES targetState);

    void clearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const FLOAT colorRGBA[4]);
    void clearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth, UINT8 stencil);

    void setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier identifier);

    void setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeapPtr descriptorHeap);

    void setCbvInDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, const Resource &resource);
    void setSrvInDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, const Resource &resource);

    void IASetVertexBuffers(UINT startSlot, UINT numBuffers, VertexBuffer *vertexBuffers);
    void IASetVertexBuffer(UINT slot, VertexBuffer &vertexBuffer);
    void IASetVertexBuffer(VertexBuffer &vertexBuffer);
    void IASetIndexBuffer(IndexBuffer &indexBuffer);
    void IASetVertexAndIndexBuffer(MeshImpl &mesh);
    void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);
    void IASetPrimitiveTopologyTriangleList();

    void RSSetViewports(UINT numViewports, const D3D12_VIEWPORT *viewports);
    void RSSetViewport(const D3D12_VIEWPORT &viewport);
    void RSSetViewport(FLOAT topLeftX, FLOAT topLeftY, FLOAT width, FLOAT height);
    void RSSetScissorRects(UINT numRects, const D3D12_RECT *rects);
    void RSSetScissorRect(const D3D12_RECT &rect);
    void RSSetScissorRectNoScissor();

    void OMSetRenderTarget(const Resource &renderTarget, const Resource &depthStencilBuffer);
    void OMSetRenderTargetDepthOnly(const Resource &depthStencilBuffer);
    void OMSetRenderTargetNoDepth(const Resource &renderTarget);

    template <typename ConstantType>
    void setGraphicsRoot32BitConstant(UINT rootParameterIndex, const ConstantType &constant);

    void drawIndexed(UINT verticesCount, INT startVertexLocation = 0u, INT startIndexLocation = 0u);
    void draw(UINT verticesCount, INT startVertexLocation = 0u);

    void close();
    void registerAllData(ResourceUsageTracker &resourceUsageTracker, uint64_t fenceValue);

    void addUsedResource(const ID3D12DescriptorHeapPtr &heap);
    void addUsedResource(const ID3D12ResourcePtr &resource);
    void addUsedResources(const ID3D12ResourcePtr *resources, UINT resourcesCount);

    auto getCommandList() { return commandList; }
    auto getDevice() const { return commandAllocatorController.getDevice(); }
    auto &getDescriptorController() const { return descriptorController; }
    auto getPipelineStateIdentifier() const { return pipelineStateIdentifier; }

private:
    void commitDescriptors();
    void commitResourceBarriers();

    // Base CommandList data
    DescriptorController &descriptorController;
    CommandAllocatorController &commandAllocatorController;
    ID3D12CommandAllocatorPtr commandAllocator;
    ID3D12GraphicsCommandListPtr commandList;

    // Descriptor controllers
    GpuDescriptorHeapController gpuDescriptorHeapControllerSampler;
    GpuDescriptorHeapController gpuDescriptorHeapControllerCbvSrvUav;

    // Tracked resources
    ID3D12DescriptorHeapPtr descriptorHeapCbvSrvUav = {};
    ID3D12DescriptorHeapPtr descriptorHeapSampler = {};
    std::set<ID3D12PageablePtr> usedResources = {};

    // Data currently set on this CommandList
    PipelineStateController::Identifier pipelineStateIdentifier;

    // Data cached to be flushed when needed
    std::vector<D3D12_RESOURCE_BARRIER> cachedResourceBarriers = {};
};

template <typename ConstantType>
inline void CommandList::setGraphicsRoot32BitConstant(UINT rootParameterIndex, const ConstantType &constant) {
    static_assert(sizeof(ConstantType) % 4 == 0, "Not a dword aligned type");
    constexpr static auto dwordCount = sizeof(ConstantType) / 4;
    commandList->SetGraphicsRoot32BitConstants(rootParameterIndex, dwordCount, &constant, 0);
}
