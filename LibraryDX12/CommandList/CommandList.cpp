#include "CommandList.h"

#include "CommandList/CommandAllocatorManager.h"
#include "Descriptor/DescriptorAllocation.h"
#include "Resource/VertexOrIndexBuffer.h"
#include "Utility/ThrowIfFailed.h"

#include <cassert>

CommandList::CommandList(DescriptorManager &descriptorManager, CommandAllocatorManager &commandAllocatorManager, ID3D12PipelineState *initialPipelineState)
    : descriptorManager(descriptorManager),
      commandAllocatorManager(commandAllocatorManager),
      commandAllocator(commandAllocatorManager.retieveCommandAllocator()),
      commandList(commandAllocatorManager.retrieveCommandList(commandAllocator, initialPipelineState)),
      gpuDescriptorHeapControllerCbvSrvUav(*this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
      gpuDescriptorHeapControllerSampler(*this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {}

CommandList::~CommandList() {
    assert(!commandAllocator && !commandList); // registerAllData should be called
}

void CommandList::transitionBarrierSingle(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter) {
    const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), stateBefore, stateAfter);
    commandList->ResourceBarrier(1, &barrier);
    addUsedResource(resource);
}

void CommandList::clearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const FLOAT colorRGBA[4]) {
    commandList->ClearRenderTargetView(renderTargetView, colorRGBA, 0, nullptr);
}

void CommandList::clearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth, UINT8 stencil) {
    commandList->ClearDepthStencilView(depthStencilView, clearFlags, depth, stencil, 0, nullptr);
}

void CommandList::setPipelineState(ID3D12PipelineStatePtr pipelineState) {
    commandList->SetPipelineState(pipelineState.Get());
}

void CommandList::setGraphicsRootSignature(ID3D12RootSignaturePtr rootSignature) {
    commandList->SetGraphicsRootSignature(rootSignature.Get());
}

void CommandList::setPipelineStateAndGraphicsRootSignature(PipelineStateController &pipelineStateController, PipelineStateController::Identifier identifier) {
    this->pipelineStateIdentifier = identifier;

    setPipelineState(pipelineStateController.getPipelineState(identifier));

    auto &rootSignature = pipelineStateController.getRootSignature(identifier);
    setGraphicsRootSignature(rootSignature.getRootSignature());
    gpuDescriptorHeapControllerCbvSrvUav.setRootSignature(rootSignature);
    gpuDescriptorHeapControllerSampler.setRootSignature(rootSignature);
}

void CommandList::setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeapPtr descriptorHeap) {
    switch (heapType) {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        descriptorHeapCbvSrvUav = descriptorHeap;
        break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
        descriptorHeapSampler = descriptorHeap;
        break;
    default:
        UNREACHABLE_CODE();
    }

    ID3D12DescriptorHeap *heaps[2];
    auto heapIndex = 0u;
    if (descriptorHeapCbvSrvUav) {
        heaps[heapIndex++] = descriptorHeapCbvSrvUav.Get();
    }
    if (descriptorHeapSampler) {
        heaps[heapIndex++] = descriptorHeapSampler.Get();
    }
    commandList->SetDescriptorHeaps(heapIndex, heaps);
    addUsedResource(descriptorHeap);
}

void CommandList::setCbvSrvUavDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, D3D12_CPU_DESCRIPTOR_HANDLE firstDescriptor, UINT descriptorCount) {
    gpuDescriptorHeapControllerCbvSrvUav.stage(rootParameterIndexOfTable, offsetInTable, firstDescriptor, descriptorCount);
}

void CommandList::setCbvSrvUavDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, const DescriptorAllocation &cpuDescriptorAllocation) {
    setCbvSrvUavDescriptorTable(rootParameterIndexOfTable, offsetInTable, cpuDescriptorAllocation.getCpuHandle(), cpuDescriptorAllocation.getHandlesCount());
}

void CommandList::setCbvSrvUavDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, const DescriptorAllocation &cpuDescriptorAllocation, UINT descriptorCount) {
    assert(descriptorCount <= cpuDescriptorAllocation.getHandlesCount());
    setCbvSrvUavDescriptorTable(rootParameterIndexOfTable, offsetInTable, cpuDescriptorAllocation.getCpuHandle(), descriptorCount);
}

void CommandList::IASetVertexBuffers(UINT startSlot, UINT numBuffers, VertexBuffer *vertexBuffers) {
    auto views = std::make_unique<D3D12_VERTEX_BUFFER_VIEW[]>(numBuffers);
    auto resources = std::make_unique<ID3D12ResourcePtr[]>(numBuffers);
    for (auto i = 0u; i < numBuffers; i++) {
        views[i] = vertexBuffers[i].getView();
        resources[i] = vertexBuffers[i].getResource();
        vertexBuffers[i].transitionBarrierSingle(*this, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    }

    commandList->IASetVertexBuffers(startSlot, numBuffers, views.get());
    addUsedResources(resources.get(), numBuffers);
}

void CommandList::IASetVertexBuffer(UINT slot, VertexBuffer &vertexBuffer) {
    vertexBuffer.transitionBarrierSingle(*this, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    commandList->IASetVertexBuffers(slot, 1, &vertexBuffer.getView());
    addUsedResource(vertexBuffer.getResource());
}

void CommandList::IASetVertexBuffer(VertexBuffer &vertexBuffer) {
    vertexBuffer.transitionBarrierSingle(*this, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    commandList->IASetVertexBuffers(0, 1, &vertexBuffer.getView());
    addUsedResource(vertexBuffer.getResource());
}

void CommandList::IASetIndexBuffer(IndexBuffer &indexBuffer) {
    indexBuffer.transitionBarrierSingle(*this, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    commandList->IASetIndexBuffer(&indexBuffer.getView());
    addUsedResource(indexBuffer.getResource());
}

void CommandList::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology) {
    commandList->IASetPrimitiveTopology(primitiveTopology);
}

void CommandList::IASetPrimitiveTopologyTriangleList() {
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void CommandList::RSSetViewports(UINT numViewports, const D3D12_VIEWPORT *viewports) {
    commandList->RSSetViewports(numViewports, viewports);
}

void CommandList::RSSetViewport(const D3D12_VIEWPORT &viewport) {
    commandList->RSSetViewports(1u, &viewport);
}

void CommandList::RSSetViewport(FLOAT topLeftX, FLOAT topLeftY, FLOAT width, FLOAT height) {
    CD3DX12_VIEWPORT viewport(topLeftX, topLeftY, width, height);
    commandList->RSSetViewports(1u, &viewport);
}

void CommandList::RSSetScissorRects(UINT numRects, const D3D12_RECT *rects) {
    commandList->RSSetScissorRects(numRects, rects);
}

void CommandList::RSSetScissorRect(const D3D12_RECT &rect) {
    commandList->RSSetScissorRects(1u, &rect);
}

void CommandList::RSSetScissorRectNoScissor() {
    CD3DX12_RECT scissorRect(0, 0, LONG_MAX, LONG_MAX);
    commandList->RSSetScissorRects(1, &scissorRect);
}

void CommandList::OMSetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE &renderTargetDescriptor, const ID3D12ResourcePtr &renderTarget,
                                    const D3D12_CPU_DESCRIPTOR_HANDLE &depthStencilDescriptor, const ID3D12ResourcePtr &depthStencilBuffer) {
    commandList->OMSetRenderTargets(1, &renderTargetDescriptor, TRUE, &depthStencilDescriptor);
    addUsedResource(renderTarget);
    addUsedResource(depthStencilBuffer);
}

void CommandList::OMSetRenderTargetDepthOnly(const D3D12_CPU_DESCRIPTOR_HANDLE &depthStencilDescriptor, const ID3D12ResourcePtr &depthStencilBuffer) {
    commandList->OMSetRenderTargets(0u, nullptr, FALSE, &depthStencilDescriptor);
    addUsedResource(depthStencilBuffer);
}

void CommandList::OMSetRenderTargetNoDepth(const D3D12_CPU_DESCRIPTOR_HANDLE &renderTargetDescriptor, const ID3D12ResourcePtr &renderTarget) {
    commandList->OMSetRenderTargets(1u, &renderTargetDescriptor, FALSE, nullptr);
    addUsedResource(renderTarget);
}

void CommandList::drawIndexed(UINT verticesCount, INT startVertexLocation, INT startIndexLocation) {
    commitDescriptors();
    commandList->DrawIndexedInstanced(verticesCount, 1, startIndexLocation, startVertexLocation, 0);
}

void CommandList::draw(UINT verticesCount, INT startIndexLocation) {
    commitDescriptors();
    commandList->DrawInstanced(verticesCount, 1, startIndexLocation, 0);
}

void CommandList::close() {
    throwIfFailed(commandList->Close());
}

void CommandList::registerAllData(ResourceUsageTracker &resourceUsageTracker, uint64_t fenceValue) {
    // Give ID3D12CommandList and ID3D12CommandAllocator back to the pool
    commandAllocatorManager.registerAllocatorAndList(commandAllocator, commandList, fenceValue);
    commandList = nullptr;
    commandAllocator = nullptr;

    // Resources reference by GPU in this CommandList
    resourceUsageTracker.registerUsage(usedResources, fenceValue);
    resourceUsageTracker.registerUsage(gpuDescriptorHeapControllerCbvSrvUav.getGpuDescriptorAllocations(), fenceValue);
    resourceUsageTracker.registerUsage(gpuDescriptorHeapControllerSampler.getGpuDescriptorAllocations(), fenceValue);
}

void CommandList::addUsedResource(const ID3D12DescriptorHeapPtr &heap) {
    usedResources.insert(heap);
}

void CommandList::addUsedResource(const ID3D12ResourcePtr &resource) {
    usedResources.insert(resource);
}

void CommandList::addUsedResources(const ID3D12ResourcePtr *resources, UINT resourcesCount) {
    usedResources.insert(resources, resources + resourcesCount);
}

void CommandList::commitDescriptors() {
    gpuDescriptorHeapControllerCbvSrvUav.commit();
    gpuDescriptorHeapControllerSampler.commit();
}
