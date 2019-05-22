#include "CommandList.h"
#include "Source/CommandAllocatorManager.h"
#include "Utility/ThrowIfFailed.h"

CommandList::CommandList(CommandAllocatorManager &commandAllocatorManager, ID3D12PipelineState *initialPipelineState)
    : commandAllocatorManager(commandAllocatorManager),
      commandAllocator(commandAllocatorManager.retieveCommandAllocator()),
      commandList(commandAllocatorManager.retrieveCommandList(commandAllocator, initialPipelineState)) {}

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

void CommandList::IASetVertexBuffers(UINT startSlot, UINT numBuffers, const D3D12_VERTEX_BUFFER_VIEW *views, const ID3D12ResourcePtr *buffers) {
    commandList->IASetVertexBuffers(startSlot, numBuffers, views);
    addUsedResources(buffers, numBuffers);
}

void CommandList::IASetVertexBuffer(UINT slot, const D3D12_VERTEX_BUFFER_VIEW &view, const ID3D12ResourcePtr &buffer) {
    commandList->IASetVertexBuffers(slot, 1, &view);
    addUsedResource(buffer);
}

void CommandList::IASetVertexBuffer(const D3D12_VERTEX_BUFFER_VIEW &view, const ID3D12ResourcePtr &buffer) {
    commandList->IASetVertexBuffers(0, 1, &view);
    addUsedResource(buffer);
}

void CommandList::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW &view, const ID3D12ResourcePtr &buffer) {
    commandList->IASetIndexBuffer(&view);
    addUsedResource(buffer);
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
    commandList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void CommandList::drawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation) {
    commandList->DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void CommandList::close() {
    throwIfFailed(commandList->Close());
}

void CommandList::addUsedResource(const ID3D12ResourcePtr &resource) {
    usedResources.insert(resource);
}

void CommandList::addUsedResources(const ID3D12ResourcePtr *resources, UINT resourcesCount) {
    usedResources.insert(resources, resources + resourcesCount);
}
