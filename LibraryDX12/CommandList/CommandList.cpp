#include "CommandList.h"

#include "CommandList/CommandAllocatorController.h"
#include "CommandList/CommandQueue.h"
#include "Descriptor/DescriptorAllocation.h"
#include "Resource/VertexOrIndexBuffer.h"
#include "Scene/MeshImpl.h"
#include "Utility/ThrowIfFailed.h"

#include <cassert>

CommandList::CommandList(CommandQueue &commandQueue, ID3D12PipelineState *initialPipelineState)
    : descriptorController(ApplicationImpl::getInstance().getDescriptorController()),
      commandAllocatorController(commandQueue.getCommandAllocatorController()),
      commandAllocator(commandAllocatorController.retrieveCommandAllocator()),
      commandList(commandAllocatorController.retrieveCommandList(commandAllocator, initialPipelineState)),
      gpuDescriptorHeapControllerCbvSrvUav(*this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
      gpuDescriptorHeapControllerSampler(*this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {}

CommandList::~CommandList() {
    assert(!commandAllocator && !commandList); // registerAllData should be called
}

void CommandList::transitionBarrier(Resource &resource, D3D12_RESOURCE_STATES targetState, UINT subresource) {
    // Update internal Resource state and compute required resource barriers
    Resource::ResourceState state = resource.getState();
    Resource::ResourceState::BarriersCreationData barriers{resource};
    state.setState(targetState, subresource, &barriers);
    resource.setState(state);

    // Push barriers to the command list
    const auto barriersStart = barriers.barriers;
    const auto barriersEnd = barriers.barriers + barriers.barriersCount;
    cachedResourceBarriers.insert(cachedResourceBarriers.end(), barriersStart, barriersEnd);
}

void CommandList::clearRenderTargetView(Resource &renderTarget, const FLOAT colorRGBA[4]) {
    commitResourceBarriers();
    commandList->ClearRenderTargetView(renderTarget.getRtv(), colorRGBA, 0, nullptr);
    addUsedResource(renderTarget.getResource());
}

void CommandList::clearDepthStencilView(Resource &dsBuffer, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth, UINT8 stencil) {
    commitResourceBarriers();
    commandList->ClearDepthStencilView(dsBuffer.getDsv(), clearFlags, depth, stencil, 0, nullptr);
    addUsedResource(dsBuffer.getResource());
}

void CommandList::setPipelineStateAndRootSignature(PipelineStateController::Identifier identifier, ResourceBindingType::ResourceBindingType resourceBindingType) {
    // Get data
    auto &pipelineStateController = ApplicationImpl::getInstance().getPipelineStateController();
    auto &pipelineState = pipelineStateController.getPipelineState(identifier);
    auto &rootSignature = pipelineStateController.getRootSignature(identifier);

    // Set pipeline state and root signature
    commandList->SetPipelineState(pipelineState.Get());
    ResourceBindingType::setRootSignatureFunctions[resourceBindingType](commandList.Get(), rootSignature.getRootSignature().Get());

    // Pass the root signature to descriptor heap controllers
    gpuDescriptorHeapControllerCbvSrvUav.setRootSignature(rootSignature);
    gpuDescriptorHeapControllerSampler.setRootSignature(rootSignature);

    // Save some state in the CommandList object
    this->pipelineStateIdentifier = identifier;
    this->resourceBindingType = resourceBindingType;
}

void CommandList::setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier identifier) {
    setPipelineStateAndRootSignature(identifier, ResourceBindingType::Graphics);
}

void CommandList::setPipelineStateAndComputeRootSignature(PipelineStateController::Identifier identifier) {
    setPipelineStateAndRootSignature(identifier, ResourceBindingType::Compute);
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

void CommandList::setCbvSrvUavInDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, const Resource &resource, D3D12_CPU_DESCRIPTOR_HANDLE descriptor) {
    gpuDescriptorHeapControllerCbvSrvUav.stage(rootParameterIndexOfTable, offsetInTable, descriptor, 1u);
    addUsedResource(resource.getResource());
}

void CommandList::setCbvInDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, const Resource &resource) {
    setCbvSrvUavInDescriptorTable(rootParameterIndexOfTable, offsetInTable, resource, resource.getCbv());
}

void CommandList::setSrvInDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, const Resource &resource) {
    setCbvSrvUavInDescriptorTable(rootParameterIndexOfTable, offsetInTable, resource, resource.getSrv());
}

void CommandList::setUavInDescriptorTable(UINT rootParameterIndexOfTable, UINT offsetInTable, const Resource &resource) {
    setCbvSrvUavInDescriptorTable(rootParameterIndexOfTable, offsetInTable, resource, resource.getUav());
}

void CommandList::IASetVertexBuffers(UINT startSlot, UINT numBuffers, VertexBuffer *vertexBuffers) {
    auto views = std::make_unique<D3D12_VERTEX_BUFFER_VIEW[]>(numBuffers);
    auto resources = std::make_unique<ID3D12ResourcePtr[]>(numBuffers);
    for (auto i = 0u; i < numBuffers; i++) {
        views[i] = vertexBuffers[i].getView();
        resources[i] = vertexBuffers[i].getResource();
        transitionBarrier(vertexBuffers[i], D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    }

    commandList->IASetVertexBuffers(startSlot, numBuffers, views.get());
    addUsedResources(resources.get(), numBuffers);
}

void CommandList::IASetVertexBuffer(UINT slot, VertexBuffer &vertexBuffer) {
    transitionBarrier(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    commandList->IASetVertexBuffers(slot, 1, &vertexBuffer.getView());
    addUsedResource(vertexBuffer.getResource());
}

void CommandList::IASetVertexBuffer(VertexBuffer &vertexBuffer) {
    transitionBarrier(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    commandList->IASetVertexBuffers(0, 1, &vertexBuffer.getView());
    addUsedResource(vertexBuffer.getResource());
}

void CommandList::IASetIndexBuffer(IndexBuffer &indexBuffer) {
    transitionBarrier(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    commandList->IASetIndexBuffer(&indexBuffer.getView());
    addUsedResource(indexBuffer.getResource());
}

void CommandList::IASetVertexAndIndexBuffer(MeshImpl &mesh) {
    IASetVertexBuffer(*mesh.getVertexBuffer());
    if (mesh.getIndicesCount() > 0u) {
        IASetIndexBuffer(*mesh.getIndexBuffer());
    }
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

void CommandList::OMSetRenderTarget(const Resource &renderTarget, const Resource &depthStencilBuffer) {
    commandList->OMSetRenderTargets(1, &renderTarget.getRtv(), TRUE, &depthStencilBuffer.getDsv());
    addUsedResource(renderTarget.getResource());
    addUsedResource(depthStencilBuffer.getResource());
}

void CommandList::OMSetRenderTargetDepthOnly(const Resource &depthStencilBuffer) {
    commandList->OMSetRenderTargets(0u, nullptr, FALSE, &depthStencilBuffer.getDsv());
    addUsedResource(depthStencilBuffer.getResource());
}

void CommandList::OMSetRenderTargetNoDepth(const Resource &renderTarget) {
    commandList->OMSetRenderTargets(1u, &renderTarget.getRtv(), FALSE, nullptr);
    addUsedResource(renderTarget.getResource());
}

void CommandList::drawIndexed(UINT verticesCount, INT startVertexLocation, INT startIndexLocation) {
    commitResourceBarriers();
    commitDescriptors();
    commandList->DrawIndexedInstanced(verticesCount, 1, startIndexLocation, startVertexLocation, 0);
}

void CommandList::draw(UINT verticesCount, INT startIndexLocation) {
    commitResourceBarriers();
    commitDescriptors();
    commandList->DrawInstanced(verticesCount, 1, startIndexLocation, 0);
}

void CommandList::dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ) {
    commitResourceBarriers();
    commitDescriptors();
    commandList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void CommandList::close() {
    commitResourceBarriers();
    throwIfFailed(commandList->Close());
}

void CommandList::registerAllData(ResourceUsageTracker &resourceUsageTracker, uint64_t fenceValue) {
    // Give ID3D12CommandList and ID3D12CommandAllocator back to the pool
    commandAllocatorController.registerAllocatorAndList(commandAllocator, commandList, fenceValue);
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
    gpuDescriptorHeapControllerCbvSrvUav.commit(resourceBindingType);
    gpuDescriptorHeapControllerSampler.commit(resourceBindingType);
}

void CommandList::commitResourceBarriers() {
    const auto barriersCount = static_cast<UINT>(cachedResourceBarriers.size());
    if (barriersCount == 0u) {
        return;
    }

    commandList->ResourceBarrier(barriersCount, cachedResourceBarriers.data());
    cachedResourceBarriers.clear();
}
