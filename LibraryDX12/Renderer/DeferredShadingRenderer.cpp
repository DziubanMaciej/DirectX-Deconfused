#include "DeferredShadingRenderer.h"

#include "CommandList/CommandList.h"
#include "Renderer/RenderData.h"
#include "Scene/CameraImpl.h"
#include "Scene/LightImpl.h"
#include "Scene/MeshImpl.h"
#include "Scene/ObjectImpl.h"
#include "Scene/SceneImpl.h"
#include "Window/SwapChain.h"

//#include "Application/ApplicationImpl.h"

DeferredShadingRenderer::DeferredShadingRenderer(SwapChain &swapChain, RenderData &renderData, SceneImpl &scene, bool shadowsEnabled)
    : swapChain(swapChain),
      renderData(renderData),
      scene(scene),
      shadowsEnabled(shadowsEnabled) {}

void DeferredShadingRenderer::renderGBuffers(CommandList &commandList) {
    commandList.RSSetViewport(0.f, 0.f, swapChain.getWidth(), swapChain.getHeight());
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    // Transition output buffers to correct states
    commandList.transitionBarrier(renderData.getGBufferAlbedo(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.transitionBarrier(renderData.getGBufferNormal(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.transitionBarrier(renderData.getGBufferSpecular(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.transitionBarrier(renderData.getDepthStencilBuffer(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

    // Clear depth buffer, gbuffers don't have to be cleared, all unwanted data from previous frame will be discarded by lighting shader
    commandList.clearDepthStencilView(renderData.getDepthStencilBuffer(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

    // View projection matrix
    const float aspectRatio = swapChain.getWidth() / swapChain.getHeight();
    scene.getCameraImpl()->setAspectRatio(aspectRatio);
    const XMMATRIX vpMatrix = scene.getCameraImpl()->getViewProjectionMatrix();

    const Resource *rts[] = {&renderData.getGBufferAlbedo(), &renderData.getGBufferNormal(), &renderData.getGBufferSpecular()};
    commandList.OMSetRenderTargets(rts, renderData.getDepthStencilBuffer());

    //Draw NORMAL
    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_NORMAL);

    for (ObjectImpl *object : scene.getObjects()) {
        MeshImpl &mesh = object->getMesh();
        if (mesh.getPipelineStateIdentifier() == commandList.getPipelineStateIdentifier()) {
            ModelMvp mmvp;
            mmvp.modelMatrix = object->getModelMatrix();
            mmvp.modelViewProjectionMatrix = XMMatrixMultiply(mmvp.modelMatrix, vpMatrix);
            commandList.setRoot32BitConstant(0, mmvp);

            ObjectPropertiesCB op = {};
            op.albedoColor = object->getColor();
            op.specularity = object->getSpecularity();
            op.bloomFactor = object->getBloomFactor();
            commandList.setRoot32BitConstant(1, op);

            commandList.IASetVertexAndIndexBuffer(mesh);
            commandList.draw(static_cast<UINT>(mesh.getVerticesCount()));
        }
    }

    //Draw TEXTURE_NORMAL
    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_TEXTURE_NORMAL);

    for (ObjectImpl *object : scene.getObjects()) {
        MeshImpl &mesh = object->getMesh();
        TextureImpl *texture = object->getTextureImpl();
        if (mesh.getPipelineStateIdentifier() == commandList.getPipelineStateIdentifier()) {
            NormalTextureCB cb;
            cb.modelMatrix = object->getModelMatrix();
            cb.modelViewProjectionMatrix = XMMatrixMultiply(cb.modelMatrix, vpMatrix);
            cb.textureScale = object->getTextureScale();
            commandList.setRoot32BitConstant(0, cb);

            ObjectPropertiesCB op = {};
            op.albedoColor = object->getColor();
            op.specularity = object->getSpecularity();
            op.bloomFactor = object->getBloomFactor();
            commandList.setRoot32BitConstant(1, op);

            commandList.IASetVertexAndIndexBuffer(mesh);
            commandList.setSrvInDescriptorTable(2, 0, *texture);
            commandList.draw(static_cast<UINT>(mesh.getVerticesCount()));
        }
    }

    //Draw TEXTURE_NORMAL_MAP
    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_TEXTURE_NORMAL_MAP);
    for (ObjectImpl *object : scene.getObjects()) {
        MeshImpl &mesh = object->getMesh();
        if (mesh.getPipelineStateIdentifier() == commandList.getPipelineStateIdentifier()) {
            TextureNormalMapCB cb;
            cb.modelMatrix = object->getModelMatrix();
            cb.modelViewProjectionMatrix = XMMatrixMultiply(cb.modelMatrix, vpMatrix);
            cb.textureScale = object->getTextureScale();
            cb.normalMapAvailable = (object->getNormalMap() != nullptr);
            commandList.setRoot32BitConstant(0, cb);

            ObjectPropertiesCB op = {};
            op.albedoColor = object->getColor();
            op.specularity = object->getSpecularity();
            op.bloomFactor = object->getBloomFactor();
            commandList.setRoot32BitConstant(1, op);

            commandList.IASetVertexAndIndexBuffer(mesh);
            if (cb.normalMapAvailable) {
                commandList.setSrvInDescriptorTable(2, 0, *object->getNormalMapImpl());
            } else {
                // TODO this is quite wasteful, maybe we can have global null descriptors?
                auto allocation = ApplicationImpl::getInstance().getDescriptorController().allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
                D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
                desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                ApplicationImpl::getInstance().getDevice()->CreateShaderResourceView(nullptr, &desc, allocation.getCpuHandle());
                commandList.setRawDescriptorInDescriptorTable(2, 0, allocation.getCpuHandle());
            }
            commandList.setSrvInDescriptorTable(2, 1, *object->getTextureImpl());
            commandList.draw(static_cast<UINT>(mesh.getVerticesCount()));
        }
    }

    commandList.transitionBarrier(renderData.getGBufferAlbedo(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getGBufferNormal(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getGBufferSpecular(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getDepthStencilBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void DeferredShadingRenderer::renderLighting(CommandList &commandList, Resource &output) {

    commandList.RSSetViewport(0.f, 0.f, swapChain.getWidth(), swapChain.getHeight());
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_LIGHTING);

    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.transitionBarrier(renderData.getBloomMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    const static FLOAT blackColor[] = {0.f, 0.f, 0.f, 1.f};
    commandList.clearRenderTargetView(renderData.getBloomMap(), blackColor);
    commandList.clearRenderTargetView(output, scene.getBackgroundColor());

    const Resource *lightingRts[] = {&renderData.getBloomMap(), &output};
    commandList.OMSetRenderTargetsNoDepth(lightingRts);

    // LightingConstantBuffer
    // TODO extract to method
    ConstantBuffer &lightConstantBuffer = renderData.getLightingConstantBuffer();
    auto lightCb = lightConstantBuffer.getData<LightingHeapCB>();
    lightCb->cameraPosition = toXmFloat4(scene.getCamera()->getEyePosition(), 0);
    lightCb->lightsSize = 0;
    lightCb->shadowMapSize = static_cast<float>(renderData.getShadowMapSize());
    lightCb->ambientLight = XMFLOAT3(scene.getAmbientLight());
    lightCb->screenWidth = swapChain.getWidth();
    lightCb->screenHeight = swapChain.getHeight();
    const auto maxLightsCount = std::min<UINT>(static_cast<UINT>(scene.getLights().size()), 8u); //  TODO dynamic light sizing
    for (; lightCb->lightsSize < maxLightsCount; lightCb->lightsSize++) {
        LightImpl &light = *scene.getLights()[lightCb->lightsSize];
        lightCb->lightColor[lightCb->lightsSize] = toXmFloat4(XMFLOAT3(light.getColor()), light.getPower());
        assert(!light.isViewOrProjectionDirty());
        lightCb->lightPosition[lightCb->lightsSize] = toXmFloat4(light.getPosition(), 1);
        lightCb->lightDirection[lightCb->lightsSize] = toXmFloat4(light.getDirection(), 0);
        if (this->shadowsEnabled) {
            lightCb->smViewProjectionMatrix[lightCb->lightsSize] = light.getShadowMapViewProjectionMatrix();
        }
        lightCb->lightsSize++;
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE lightConstantBufferView = lightConstantBuffer.uploadAndSwap();

    commandList.setCbvSrvUavInDescriptorTable(0, 0, lightConstantBuffer, lightConstantBufferView); // TODO set null descriptor if shadows are off
    commandList.setSrvInDescriptorTable(0, 1, renderData.getGBufferAlbedo());
    commandList.setSrvInDescriptorTable(0, 2, renderData.getGBufferNormal());
    commandList.setSrvInDescriptorTable(0, 3, renderData.getGBufferSpecular());
    commandList.setSrvInDescriptorTable(0, 4, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 5, renderData.getSsaoMap());
    for (auto shadowMapIndex = 0u; shadowMapIndex < 8; shadowMapIndex++) {
        commandList.setSrvInDescriptorTable(0, shadowMapIndex + 6, renderData.getShadowMap(shadowMapIndex));
    }

    LightingCB lcb;
    lcb.viewMatrixInverse = scene.getCameraImpl()->getInvViewMatrix();
    lcb.projMatrixInverse = scene.getCameraImpl()->getInvProjectionMatrix();
    lcb.enableSSAO = ApplicationImpl::getInstance().getSettings().getSsaoEnabled();
    commandList.setRoot32BitConstant(1, lcb);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);
}
