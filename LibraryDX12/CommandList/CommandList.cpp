#include "CommandList.h"

#include "CommandList/CommandAllocatorManager.h"
#include "Descriptor/CpuDescriptorAllocation.h"
#include "Resource/VertexOrIndexBuffer.h"
#include "Utility/ThrowIfFailed.h"

#include <cassert>

CommandList::CommandList(CommandAllocatorManager &commandAllocatorManager, ID3D12PipelineState *initialPipelineState)
    : commandAllocatorManager(commandAllocatorManager),
      commandAllocator(commandAllocatorManager.retieveCommandAllocator()),
      commandList(commandAllocatorManager.retrieveCommandList(commandAllocator, initialPipelineState)),
      gpuDescriptorHeapControllerCbvSrvUav(*this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
      gpuDescriptorHeapControllerSampler(*this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {}

CommandList::~CommandList() {
    commandAllocatorManager.registerAllocatorAndList(commandAllocator, commandList, fenceValue);
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
    setPipelineState(pipelineStateController.getPipelineState(identifier));

    auto rootSignature = pipelineStateController.getRootSignature(identifier);
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
        unreachableCode();
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

void CommandList::setCbvSrvUavDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, const CpuDescriptorAllocation &cpuDescriptorAllocation) {
    setCbvSrvUavDescriptorTable(rootParameterIndexOfTable, offsetInTable, cpuDescriptorAllocation.getCpuHandle(), cpuDescriptorAllocation.getHandlesCount());
}

void CommandList::setCbvSrvUavDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, const CpuDescriptorAllocation &cpuDescriptorAllocation, UINT descriptorCount) {
    assert(descriptorCount <= cpuDescriptorAllocation.getHandlesCount());
    setCbvSrvUavDescriptorTable(rootParameterIndexOfTable, offsetInTable, cpuDescriptorAllocation.getCpuHandle(), descriptorCount);
}

void CommandList::IASetVertexBuffers(UINT startSlot, UINT numBuffers, const VertexBuffer *vertexBuffers) {
    auto views = std::make_unique<D3D12_VERTEX_BUFFER_VIEW[]>(numBuffers);
    auto resources = std::make_unique<ID3D12ResourcePtr[]>(numBuffers);
    for (auto i = 0u; i < numBuffers; i++) {
        views[i] = vertexBuffers[i].getView();
        resources[i] = vertexBuffers[i].getResource();
    }

    commandList->IASetVertexBuffers(startSlot, numBuffers, views.get());
    addUsedResources(resources.get(), numBuffers);
}

void CommandList::IASetVertexBuffer(UINT slot, const VertexBuffer &vertexBuffer) {
    commandList->IASetVertexBuffers(slot, 1, &vertexBuffer.getView());
    addUsedResource(vertexBuffer.getResource());
}

void CommandList::IASetVertexBuffer(const VertexBuffer &vertexBuffer) {
    commandList->IASetVertexBuffers(0, 1, &vertexBuffer.getView());
    addUsedResource(vertexBuffer.getResource());
}

void CommandList::IASetIndexBuffer(const IndexBuffer &indexBuffer) {
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
    commandList->RSSetViewports(1, &viewport);
}

void CommandList::RSSetScissorRects(UINT numRects, const D3D12_RECT *rects) {
    commandList->RSSetScissorRects(numRects, rects);
}

void CommandList::RSSetScissorRect(const D3D12_RECT &rect) {
    commandList->RSSetScissorRects(1, &rect);
}

void CommandList::OMSetRenderTargets(UINT renderTargetsCount, const D3D12_CPU_DESCRIPTOR_HANDLE *renderTargetDescriptors,
                                     const ID3D12ResourcePtr *renderTargets, BOOL singleHandleToDescriptorRange,
                                     const D3D12_CPU_DESCRIPTOR_HANDLE &depthStencilDescriptor, const ID3D12ResourcePtr &depthStencilBuffer) {
    commandList->OMSetRenderTargets(renderTargetsCount, renderTargetDescriptors, singleHandleToDescriptorRange, &depthStencilDescriptor);
    addUsedResources(renderTargets, renderTargetsCount);
    addUsedResource(depthStencilBuffer);
}

void CommandList::OMSetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE &renderTargetDescriptor, const ID3D12ResourcePtr &renderTarget,
                                    const D3D12_CPU_DESCRIPTOR_HANDLE &depthStencilDescriptor, const ID3D12ResourcePtr &depthStencilBuffer) {
    commandList->OMSetRenderTargets(1, &renderTargetDescriptor, TRUE, &depthStencilDescriptor);
    addUsedResource(renderTarget);
    addUsedResource(depthStencilBuffer);
}

void CommandList::drawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation) {
    commitDescriptors();
    commandList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void CommandList::drawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation) {
    commitDescriptors();
    commandList->DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void CommandList::drawInstanced(UINT vertexCountPerInstance, UINT instanceCount, INT baseVertexLocation, UINT startInstanceLocation) {
    commitDescriptors();
    commandList->DrawInstanced(vertexCountPerInstance, instanceCount, baseVertexLocation, startInstanceLocation);
}

void CommandList::close() {
    throwIfFailed(commandList->Close());
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
