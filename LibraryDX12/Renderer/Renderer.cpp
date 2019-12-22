#include "Renderer.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "CommandList/CommandListInserter.h"
#include "CommandList/CommandQueue.h"
#include "ConstantBuffers/ConstantBuffers.h"
#include "Renderer/RenderData.h"
#include "Scene/CameraImpl.h"
#include "Scene/LightImpl.h"
#include "Scene/ObjectImpl.h"
#include "Scene/SceneImpl.h"
#include "Scene/SpriteImpl.h"
#include "Scene/TextImpl.h"
#include "Utility/DxObjectNaming.h"
#include "Utility/ThrowIfFailed.h"
#include "Window/WindowImpl.h"

#include <ExternalHeaders/Wrappers/d3dx12.h>
#include <algorithm>
#include <cassert>

// --------------------------------------------------------------------------- Render methods

Renderer::Renderer(SwapChain &swapChain, RenderData &renderData, SceneImpl &scene)
    : swapChain(swapChain),
      renderData(renderData),
      scene(scene),
      shadowsRenderer(swapChain, renderData, scene),
      deferredShadingRenderer(swapChain, renderData, scene, shadowsRenderer.isEnabled()),
      postProcessRenderer(renderData, scene.getPostProcesses(), swapChain.getWidth(), swapChain.getHeight()),
      spriteRenderer(swapChain, renderData, scene),
      textRenderer(scene) {}

void Renderer::renderSSAO(CommandList &commandList) {
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(std::max(swapChain.getWidthUint() / 2, 1u)), static_cast<float>(std::max(swapChain.getHeightUint() / 2, 1u)));
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.transitionBarrier(renderData.getSsaoMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SSAO);

    commandList.OMSetRenderTargetNoDepth(renderData.getSsaoMap());

    commandList.setSrvInDescriptorTable(0, 0, renderData.getGBufferNormal());
    commandList.setSrvInDescriptorTable(0, 1, renderData.getDepthStencilBuffer());

    SsaoCB ssaoCB;
    ssaoCB.screenWidth = static_cast<float>(std::max(swapChain.getWidthUint() / 2, 1u));
    ssaoCB.screenHeight = static_cast<float>(std::max(swapChain.getHeightUint() / 2, 1u));
    ssaoCB.viewMatrixInverse = scene.getCameraImpl()->getInvViewMatrix();
    ssaoCB.projMatrixInverse = scene.getCameraImpl()->getInvProjectionMatrix();
    commandList.setRoot32BitConstant(1, ssaoCB);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);

    commandList.transitionBarrier(renderData.getSsaoMap(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void Renderer::renderSSRandMerge(CommandList &commandList, Resource &input, Resource &output) {
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(std::max(swapChain.getWidthUint(), 1u)), static_cast<float>(std::max(swapChain.getHeightUint(), 1u)));
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SSR);
    commandList.transitionBarrier(input, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getSsrMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.OMSetRenderTargetNoDepth(renderData.getSsrMap());

    commandList.setSrvInDescriptorTable(0, 0, renderData.getGBufferNormal());
    commandList.setSrvInDescriptorTable(0, 1, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 2, renderData.getGBufferSpecular());
    commandList.setSrvInDescriptorTable(0, 3, input);

    SsrCB ssrCB;
    CameraImpl &camera = *scene.getCameraImpl();
    ssrCB.cameraPosition = XMFLOAT4(camera.getEyePosition().x, camera.getEyePosition().y, camera.getEyePosition().z, 1);
    ssrCB.screenWidth = static_cast<float>(std::max(swapChain.getWidthUint() / 2, 1u));
    ssrCB.screenHeight = static_cast<float>(std::max(swapChain.getHeightUint() / 2, 1u));
    ssrCB.viewMatrixInverse = camera.getInvViewMatrix();
    ssrCB.projMatrixInverse = camera.getInvProjectionMatrix();
    ssrCB.viewProjectionMatrix = camera.getViewProjectionMatrix();
    ssrCB.clearColor = toXmFloat4(XMFLOAT3(scene.getBackgroundColor()), 1.0f);
    commandList.setRoot32BitConstant(1, ssrCB);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);

    // SSR Blur
    commandList.RSSetViewport(0.f, 0.f, swapChain.getWidth(), swapChain.getHeight());
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SSR_BLUR);

    commandList.transitionBarrier(renderData.getSsrMap(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getSsrBlurredMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.OMSetRenderTargetNoDepth(renderData.getSsrBlurredMap());

    commandList.setSrvInDescriptorTable(0, 0, renderData.getGBufferSpecular());
    commandList.setSrvInDescriptorTable(0, 1, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 2, renderData.getSsrMap());

    SsrMergeCB ssrMergeCB;
    ssrMergeCB.screenWidth = swapChain.getWidth();
    ssrMergeCB.screenHeight = swapChain.getHeight();
    commandList.setRoot32BitConstant(1, ssrMergeCB);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);

    // SSR Merge
    commandList.RSSetViewport(0.f, 0.f, swapChain.getWidth(), swapChain.getHeight());
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SSR_MERGE);

    commandList.transitionBarrier(renderData.getSsrBlurredMap(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.OMSetRenderTargetNoDepth(output);

    commandList.setSrvInDescriptorTable(0, 0, renderData.getGBufferSpecular());
    commandList.setSrvInDescriptorTable(0, 1, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 2, renderData.getSsrBlurredMap());
    commandList.setSrvInDescriptorTable(0, 3, input);

    commandList.setRoot32BitConstant(1, ssrMergeCB);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    ///commandList.getCommandList()->EndQuery(queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0);

    commandList.draw(6u);

    ///commandList.getCommandList()->EndQuery(queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 1);
    ///commandList.getCommandList()->ResolveQueryData(queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, 2, queryResult->getResource().Get(), 0);
}

void Renderer::renderFog(CommandList &commandList, Resource &input, Resource &output) {
    commandList.RSSetViewport(0.f, 0.f, swapChain.getWidth(), swapChain.getHeight());
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.transitionBarrier(input, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_FOG);

    commandList.OMSetRenderTargetNoDepth(output);

    commandList.setSrvInDescriptorTable(0, 0, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 1, input);

    FogCB fogCB;
    fogCB.screenWidth = swapChain.getWidth();
    fogCB.screenHeight = swapChain.getHeight();
    fogCB.fogPower = scene.getFogPower();
    fogCB.fogColor = XMFLOAT3(scene.getFogColor());
    fogCB.viewMatrixInverse = scene.getCameraImpl()->getInvViewMatrix();
    fogCB.projMatrixInverse = scene.getCameraImpl()->getInvProjectionMatrix();
    fogCB.cameraPosition = toXmFloat4(scene.getCameraImpl()->getEyePosition(), 1);
    commandList.setRoot32BitConstant(1, fogCB);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);
}

void Renderer::renderDof(CommandList &commandList, Resource &input, Resource &output) {
    // DOF
    commandList.RSSetViewport(0.f, 0.f, swapChain.getWidth(), swapChain.getHeight());
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.transitionBarrier(input, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getDofMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_DOF_BLUR);

    commandList.OMSetRenderTargetNoDepth(renderData.getDofMap());

    commandList.setSrvInDescriptorTable(0, 0, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 1, input);

    DofCB dofCB;
    dofCB.screenWidth = swapChain.getWidth();
    dofCB.screenHeight = swapChain.getHeight();
    commandList.setRoot32BitConstant(1, dofCB);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);

    // DOF 2
    commandList.RSSetViewport(0.f, 0.f, swapChain.getWidth(), swapChain.getHeight());
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.transitionBarrier(renderData.getDofMap(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_DOF);

    commandList.OMSetRenderTargetNoDepth(output);

    commandList.setSrvInDescriptorTable(0, 0, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 1, renderData.getDofMap());

    commandList.setRoot32BitConstant(1, dofCB);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);
}

void Renderer::copyToBackBuffer(CommandList &commandList, Resource &source) {
    auto &backBuffer = swapChain.getCurrentBackBuffer();
    commandList.transitionBarrier(source, D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList.transitionBarrier(backBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
    commandList.copyResource(backBuffer, source);
}

void Renderer::render() {
    auto &application = ApplicationImpl::getInstance();
    auto &commandQueue = application.getDirectCommandQueue();
    auto &backBuffer = swapChain.getCurrentBackBuffer();
    auto &alternatingResources = renderData.getSceneAlternatingResources();
    application.flushAllResources();
    CommandListInserter commandListInserter{commandQueue};

    // Render shadow maps
    if (shadowsRenderer.isEnabled()) {
        shadowsRenderer.renderShadowMaps(commandListInserter.currentList());
        commandListInserter.submitLists();
    }

    // GBuffer
    commandListInserter.newList();
    deferredShadingRenderer.renderGBuffers(commandListInserter.currentList());
    commandListInserter.submitLists();

    // SSAO
    if (ApplicationImpl::getInstance().getSettings().getSsaoEnabled()) {
        renderSSAO(commandListInserter.currentList());
    }

    // Lighting - merge GBuffers and apply per-pixel shading
    deferredShadingRenderer.renderLighting(commandListInserter.currentList(), alternatingResources.getDestination());
    alternatingResources.swapResources();

    // Bloom
    if (ApplicationImpl::getInstance().getSettings().getBloomEnabled()) {
        Resource &source = renderData.getBloomMap();
        Resource &destination = alternatingResources.getSource(); // render to previously rendered frame
        postProcessRenderer.renderBloom(commandListInserter.currentList(), source, destination);
    }

    // SSR
    if (ApplicationImpl::getInstance().getSettings().getSsrEnabled()) {
        Resource &source = alternatingResources.getSource();
        Resource &destination = alternatingResources.getDestination();
        renderSSRandMerge(commandListInserter.currentList(), source, destination);
        alternatingResources.swapResources();
    }

    // Fog
    if (ApplicationImpl::getInstance().getSettings().getFogEnabled()) {
        Resource &source = alternatingResources.getSource();
        Resource &destination = alternatingResources.getDestination();
        renderFog(commandListInserter.currentList(), source, destination);
        alternatingResources.swapResources();
    }

    // Dof
    if (ApplicationImpl::getInstance().getSettings().getDofEnabled()) {
        Resource &source = alternatingResources.getSource();
        Resource &destination = alternatingResources.getDestination();
        renderDof(commandListInserter.currentList(), source, destination);
        alternatingResources.swapResources();
    }

    // Send to GPU
    commandListInserter.submitLists();

    // Render post processes
    postProcessRenderer.renderPostProcesses(commandListInserter.currentList(), alternatingResources, swapChain.getWidth(), swapChain.getHeight());
    copyToBackBuffer(commandListInserter.currentList(), alternatingResources.getSource()); // resource has been swapped, so taking the "source"

    // Render sprites
    spriteRenderer.renderSprites(commandListInserter.currentList(), backBuffer);

    // Prepare back buffer state
    if (textRenderer.isEnabled()) {
        // Text rendering requires resource to be in this state and will transition it to PRESENT afterwards
        commandListInserter.currentList().transitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    } else {
        // These are the last rendering commands, will be presenting now
        commandListInserter.currentList().transitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
    }

    // Send to GPU
    commandListInserter.submitLists();

    // Render on-screen texts with D2D. Commands will be immediately submitted to the queue
    if (textRenderer.isEnabled()) {
        ApplicationImpl::getInstance().getD2DContext(); // Initialize context
        textRenderer.renderTexts(swapChain.getCurrentD2DWrappedBackBuffer());
    }

    // Present (swap back buffers) and wait for next frame's fence
    const uint64_t fenceValue = commandQueue.signalOnGpu();
    swapChain.present(fenceValue);
    commandQueue.waitOnCpu(swapChain.getFenceValueForCurrentBackBuffer());
}
