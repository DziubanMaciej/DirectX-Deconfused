#include "ShadowsRenderer.h"

#include "Application/ApplicationImpl.h"
#include "Renderer/RenderData.h"
#include "Scene/CameraImpl.h"
#include "Scene/LightImpl.h"
#include "Scene/MeshImpl.h"
#include "Scene/ObjectImpl.h"
#include "Scene/SceneImpl.h"

ShadowsRenderer::ShadowsRenderer(SwapChain &swapChain, RenderData &renderData, SceneImpl &scene)
    : swapChain(swapChain),
      renderData(renderData),
      scene(scene),
      enabled(ApplicationImpl::getInstance().getSettings().getShadowsQuality() > 0) {}

void ShadowsRenderer::renderShadowMaps(CommandList &commandList) {
    const auto &lights = scene.getLights();
    const auto shadowMapsUsed = std::min(size_t{8u}, lights.size());

    for (int i = 0; i < shadowMapsUsed; i++) {
        commandList.transitionBarrier(renderData.getShadowMap(i), D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }

    const auto shadowMapSize = static_cast<float>(renderData.getShadowMapSize());
    commandList.RSSetViewport(0.f, 0.f, shadowMapSize, shadowMapSize);
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    int lightIdx = 0;

    for (LightImpl *light : lights) {
        commandList.OMSetRenderTargetDepthOnly(renderData.getShadowMap(lightIdx));
        commandList.clearDepthStencilView(renderData.getShadowMap(lightIdx), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

        // View projection matrix
        scene.getCameraImpl()->setAspectRatio(1.0f);
        const XMMATRIX smViewProjectionMatrix = light->getShadowMapViewProjectionMatrix();

        // Draw NORMAL
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SM_NORMAL);
        for (ObjectImpl *object : scene.getObjects()) {
            MeshImpl &mesh = object->getMesh();
            if (mesh.getShadowMapPipelineStateIdentifier() == commandList.getPipelineStateIdentifier()) {
                ShadowMapCB cb;
                cb.mvp = XMMatrixMultiply(object->getModelMatrix(), smViewProjectionMatrix);
                commandList.setRoot32BitConstant(0, cb);

                commandList.IASetVertexAndIndexBuffer(mesh);
                commandList.draw(static_cast<UINT>(mesh.getVerticesCount()));
            }
        }

        // Draw TEXTURE NORMAL
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SM_TEXTURE_NORMAL);
        for (ObjectImpl *object : scene.getObjects()) {
            MeshImpl &mesh = object->getMesh();
            if (mesh.getShadowMapPipelineStateIdentifier() == commandList.getPipelineStateIdentifier()) {
                ShadowMapCB cb;
                cb.mvp = XMMatrixMultiply(object->getModelMatrix(), smViewProjectionMatrix);
                commandList.setRoot32BitConstant(0, cb);

                commandList.IASetVertexAndIndexBuffer(mesh);
                commandList.draw(static_cast<UINT>(mesh.getVerticesCount()));
            }
        }

        // Draw TEXTURE NORMAL MAP
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SM_TEXTURE_NORMAL_MAP);
        for (ObjectImpl *object : scene.getObjects()) {
            MeshImpl &mesh = object->getMesh();
            if (mesh.getShadowMapPipelineStateIdentifier() == commandList.getPipelineStateIdentifier()) {
                ShadowMapCB cb;
                cb.mvp = XMMatrixMultiply(object->getModelMatrix(), smViewProjectionMatrix);
                commandList.setRoot32BitConstant(0, cb);

                commandList.IASetVertexAndIndexBuffer(mesh);
                commandList.draw(static_cast<UINT>(mesh.getVerticesCount()));
            }
        }

        lightIdx++;
    }

    for (int i = 0; i < shadowMapsUsed; i++) {
        commandList.transitionBarrier(renderData.getShadowMap(i), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
}
