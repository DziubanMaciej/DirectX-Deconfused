#include "SceneImpl.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "CommandList/CommandQueue.h"
#include "ConstantBuffers/ConstantBuffers.h"
#include "Scene/RenderData.h"
#include "Utility/DxObjectNaming.h"
#include "Utility/ThrowIfFailed.h"
#include "Window/WindowImpl.h"

#include <ExternalHeaders/Wrappers/d3dx12.h>
#include <algorithm>
#include <cassert>

namespace DXD {
std::unique_ptr<Scene> Scene::create() {
    return std::unique_ptr<Scene>{new SceneImpl()};
}
} // namespace DXD

SceneImpl::SceneImpl() {
    auto device = ApplicationImpl::getInstance().getDevice();

    // QUERY!
    D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
    queryHeapDesc.Count = 2;
    queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    queryHeapDesc.NodeMask = 0;
    throwIfFailed(device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&queryHeap)));

    // query desc
    D3D12_RESOURCE_DESC queryDestDesc = {};
    queryDestDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    queryDestDesc.Alignment = 0;
    queryDestDesc.Width = 2 * sizeof(UINT64);
    queryDestDesc.Height = 1;
    queryDestDesc.DepthOrArraySize = 1;
    queryDestDesc.MipLevels = 1;
    queryDestDesc.Format = DXGI_FORMAT_UNKNOWN;
    queryDestDesc.SampleDesc.Count = 1;
    queryDestDesc.SampleDesc.Quality = 0;
    queryDestDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    queryDestDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    heapProps.Type = D3D12_HEAP_TYPE_CUSTOM;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

    queryResult = std::make_unique<Resource>(
        device,
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &queryDestDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr);
}

// --------------------------------------------------------------------------- Global colors

void SceneImpl::setBackgroundColor(float r, float g, float b) {
    backgroundColor[0] = r;
    backgroundColor[1] = g;
    backgroundColor[2] = b;
}

void SceneImpl::setAmbientLight(float r, float g, float b) {
    ambientLight[0] = r;
    ambientLight[1] = g;
    ambientLight[2] = b;
}

void SceneImpl::setFogColor(float r, float g, float b) {
    fogColor[0] = r;
    fogColor[1] = g;
    fogColor[2] = b;
}

void SceneImpl::setFogPower(float pow) {
    fogPower = pow;
}

// --------------------------------------------------------------------------- Scene management

void SceneImpl::addLight(DXD::Light &light) {
    lights.push_back(static_cast<LightImpl *>(&light));
}

unsigned int SceneImpl::removeLight(DXD::Light &light) {
    return removeFromScene(lights, light);
}

void SceneImpl::addPostProcess(DXD::PostProcess &postProcess) {
    postProcesses.push_back(static_cast<PostProcessImpl *>(&postProcess));
}

unsigned int SceneImpl::removePostProcess(DXD::PostProcess &postProcess) {
    return removeFromScene(postProcesses, postProcess);
}

void SceneImpl::addSprite(DXD::Sprite &sprite) {
    sprites.push_back(static_cast<SpriteImpl *>(&sprite));
}

unsigned int SceneImpl::removeSprite(DXD::Sprite &sprite) {
    return removeFromScene(sprites, sprite);
}

void SceneImpl::addText(DXD::Text &text) {
    texts.push_back(static_cast<TextImpl *>(&text));
}

unsigned int SceneImpl::removeText(DXD::Text &text) {
    return removeFromScene(texts, text);
}

void SceneImpl::addObject(DXD::Object &object) {
    objectsNotReady.insert(static_cast<ObjectImpl *>(&object));
}

unsigned int SceneImpl::removeObject(DXD::Object &object) {
    const auto toErase = static_cast<ObjectImpl *>(&object);
    const auto elementsRemoved = objects.erase(toErase) + objectsNotReady.erase(toErase);
    assert(elementsRemoved == 0u || elementsRemoved == 1u);
    return elementsRemoved == 1u;
}

void SceneImpl::setCamera(DXD::Camera &camera) {
    this->camera = static_cast<CameraImpl *>(&camera);
}

DXD::Camera *SceneImpl::getCamera() {
    return camera;
}

// ---------------------------------------------------------------------------  Helpers

void SceneImpl::inspectObjectsNotReady() {
    for (auto it = objectsNotReady.begin(); it != objectsNotReady.end();) {
        ObjectImpl *object = *it;
        if (object->isReady()) {
            auto itToDelete = it++;
            objects.insert(object);
            objectsNotReady.erase(itToDelete);
        } else {
            it++;
        }
    }
}

size_t SceneImpl::getEnabledPostProcessesCount() const {
    size_t count = 0u;
    for (const auto &postProcess : this->postProcesses) {
        if (postProcess->isEnabled()) {
            count++;
        }
    }
    return count;
}

void SceneImpl::getSourceAndDestinationForPostProcess(AlternatingResources &alternatingResources, Resource *optionalInput,
                                                      Resource *&outSource, Resource *&outDestination) {
    outSource = (optionalInput != nullptr) ? optionalInput : &alternatingResources.getSource();
    outDestination = &alternatingResources.getDestination();
}

void SceneImpl::prepareSourceAndDestinationForPostProcess(CommandList &commandList, Resource &source, Resource &destination, bool compute) {
    if (compute) {
        commandList.transitionBarrier(source, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        commandList.transitionBarrier(destination, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    } else {
        commandList.transitionBarrier(source, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        commandList.transitionBarrier(destination, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
}

// --------------------------------------------------------------------------- Render methods

const static FLOAT blackColor[] = {0.f, 0.f, 0.f, 1.f};

void SceneImpl::renderShadowMaps(SwapChain &swapChain, RenderData &renderData, CommandList &commandList) {
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
        camera->setAspectRatio(1.0f);
        const XMMATRIX smViewProjectionMatrix = light->getShadowMapViewProjectionMatrix();

        // Light values used in Lighting
        lightVpMatrixes.push_back(smViewProjectionMatrix);
        lightPositions.push_back(XMFLOAT4(light->getPosition().x, light->getPosition().y, light->getPosition().z, 0));
        lightDirections.push_back(XMFLOAT4(light->getDirection().x, light->getDirection().y, light->getDirection().z, 0));

        // Draw NORMAL
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SM_NORMAL);
        for (ObjectImpl *object : objects) {
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
        for (ObjectImpl *object : objects) {
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

void SceneImpl::renderGBuffer(SwapChain &swapChain, RenderData &renderData, CommandList &commandList) {
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
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
    const float aspectRatio = static_cast<float>(swapChain.getWidth()) / static_cast<float>(swapChain.getHeight());
    camera->setAspectRatio(aspectRatio);
    const XMMATRIX vpMatrix = camera->getViewProjectionMatrix();

    const Resource *rts[] = {&renderData.getGBufferAlbedo(), &renderData.getGBufferNormal(), &renderData.getGBufferSpecular()};
    commandList.OMSetRenderTargets(rts, renderData.getDepthStencilBuffer());

    //Draw NORMAL
    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_NORMAL);

    for (ObjectImpl *object : objects) {
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

    for (ObjectImpl *object : objects) {
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
    for (ObjectImpl *object : objects) {
        MeshImpl &mesh = object->getMesh();
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
            commandList.setSrvInDescriptorTable(2, 0, *object->getNormalMapImpl());
            commandList.setSrvInDescriptorTable(2, 1, *object->getTextureImpl());
            commandList.draw(static_cast<UINT>(mesh.getVerticesCount()));
        }
    }

    commandList.transitionBarrier(renderData.getGBufferAlbedo(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getGBufferNormal(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getGBufferSpecular(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getDepthStencilBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void SceneImpl::renderSSAO(SwapChain &swapChain, RenderData &renderData, CommandList &commandList) {
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(std::max(swapChain.getWidth() / 2, 1u)), static_cast<float>(std::max(swapChain.getHeight() / 2, 1u)));
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.transitionBarrier(renderData.getSsaoMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SSAO);

    const Resource *ssaoRts[] = {&renderData.getSsaoMap()};
    commandList.OMSetRenderTargetsNoDepth(ssaoRts);

    commandList.setSrvInDescriptorTable(0, 0, renderData.getGBufferNormal());
    commandList.setSrvInDescriptorTable(0, 1, renderData.getDepthStencilBuffer());

    SsaoCB ssaoCB;
    ssaoCB.screenWidth = static_cast<float>(std::max(swapChain.getWidth() / 2, 1u));
    ssaoCB.screenHeight = static_cast<float>(std::max(swapChain.getHeight() / 2, 1u));
    ssaoCB.viewMatrixInverse = camera->getInvViewMatrix();
    ssaoCB.projMatrixInverse = camera->getInvProjectionMatrix();
    commandList.setRoot32BitConstant(1, ssaoCB);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);

    commandList.transitionBarrier(renderData.getSsaoMap(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void SceneImpl::renderLighting(SwapChain &swapChain, RenderData &renderData, CommandList &commandList, Resource &output) {

    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_LIGHTING);

    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.transitionBarrier(renderData.getBloomMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.clearRenderTargetView(renderData.getBloomMap(), blackColor);
    commandList.clearRenderTargetView(output, backgroundColor);

    const Resource *lightingRts[] = {&renderData.getBloomMap(), &output};
    commandList.OMSetRenderTargetsNoDepth(lightingRts);

    // LightingConstantBuffer
    ConstantBuffer &lightConstantBuffer = renderData.getLightingConstantBuffer();
    auto lightCb = lightConstantBuffer.getData<LightingHeapCB>();
    lightCb->cameraPosition = XMFLOAT4(camera->getEyePosition().x, camera->getEyePosition().y, camera->getEyePosition().z, 1);
    lightCb->lightsSize = 0;
    lightCb->shadowMapSize = static_cast<float>(renderData.getShadowMapSize());
    lightCb->ambientLight = XMFLOAT3(ambientLight[0], ambientLight[1], ambientLight[2]);
    lightCb->screenWidth = static_cast<float>(swapChain.getWidth());
    lightCb->screenHeight = static_cast<float>(swapChain.getHeight());
    for (LightImpl *light : lights) {
        lightCb->lightColor[lightCb->lightsSize] = XMFLOAT4(light->getColor().x, light->getColor().y, light->getColor().z, light->getPower());
        lightCb->lightPosition[lightCb->lightsSize] = lightPositions[lightCb->lightsSize];
        lightCb->lightDirection[lightCb->lightsSize] = lightDirections[lightCb->lightsSize];
        lightCb->smViewProjectionMatrix[lightCb->lightsSize] = lightVpMatrixes[lightCb->lightsSize];
        lightCb->lightsSize++;
        if (lightCb->lightsSize >= 8) {
            break;
        }
    }
    lightConstantBuffer.upload();
    lightVpMatrixes.clear();
    lightPositions.clear();
    lightDirections.clear();


    commandList.setCbvInDescriptorTable(0, 0, lightConstantBuffer);
    commandList.setSrvInDescriptorTable(0, 1, renderData.getGBufferAlbedo());
    commandList.setSrvInDescriptorTable(0, 2, renderData.getGBufferNormal());
    commandList.setSrvInDescriptorTable(0, 3, renderData.getGBufferSpecular());
    commandList.setSrvInDescriptorTable(0, 4, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 5, renderData.getSsaoMap());
    for (auto shadowMapIndex = 0u; shadowMapIndex < 8; shadowMapIndex++) {
        commandList.setSrvInDescriptorTable(0, shadowMapIndex + 6, renderData.getShadowMap(shadowMapIndex));
    }

    LightingCB lcb;
    lcb.viewMatrixInverse = camera->getInvViewMatrix();
    lcb.projMatrixInverse = camera->getInvProjectionMatrix();
    lcb.enableSSAO = ApplicationImpl::getInstance().getSettings().getSsaoEnabled();
    commandList.setRoot32BitConstant(1, lcb);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);
}

void SceneImpl::renderSSRandMerge(SwapChain &swapChain, RenderData &renderData, CommandList &commandList, Resource &input, Resource &output) {
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(std::max(swapChain.getWidth(), 1u)), static_cast<float>(std::max(swapChain.getHeight(), 1u)));
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SSR);
    commandList.transitionBarrier(input, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getSsrMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    const Resource *ssrRts[] = {&renderData.getSsrMap()};
    commandList.OMSetRenderTargetsNoDepth(ssrRts);

    commandList.setSrvInDescriptorTable(0, 0, renderData.getGBufferNormal());
    commandList.setSrvInDescriptorTable(0, 1, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 2, renderData.getGBufferSpecular());
    commandList.setSrvInDescriptorTable(0, 3, input);

    SsrCB ssrCB;
    ssrCB.cameraPosition = XMFLOAT4(camera->getEyePosition().x, camera->getEyePosition().y, camera->getEyePosition().z, 1);
    ssrCB.screenWidth = static_cast<float>(std::max(swapChain.getWidth() / 2, 1u));
    ssrCB.screenHeight = static_cast<float>(std::max(swapChain.getHeight() / 2, 1u));
    ssrCB.viewMatrixInverse = camera->getInvViewMatrix();
    ssrCB.projMatrixInverse = camera->getInvProjectionMatrix();
    ssrCB.viewProjectionMatrix = camera->getViewProjectionMatrix();
    ssrCB.clearColor = XMFLOAT4(backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.0f);
    commandList.setRoot32BitConstant(1, ssrCB);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);

    // SSR Blur
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SSR_BLUR);

    commandList.transitionBarrier(renderData.getSsrMap(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getSsrBlurredMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    const Resource *ssrMergeRts[] = {&renderData.getSsrBlurredMap()};
    commandList.OMSetRenderTargetsNoDepth(ssrMergeRts);

    commandList.setSrvInDescriptorTable(0, 0, renderData.getGBufferSpecular());
    commandList.setSrvInDescriptorTable(0, 1, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 2, renderData.getSsrMap());

    SsrMergeCB ssrMergeCB;
    ssrMergeCB.screenWidth = static_cast<float>(swapChain.getWidth());
    ssrMergeCB.screenHeight = static_cast<float>(swapChain.getHeight());
    commandList.setRoot32BitConstant(1, ssrMergeCB);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);

    // SSR Merge
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SSR_MERGE);

    commandList.transitionBarrier(renderData.getSsrBlurredMap(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);

    const Resource *ssrBlurRts[] = {&output};
    commandList.OMSetRenderTargetsNoDepth(ssrBlurRts);

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

void SceneImpl::renderFog(SwapChain &swapChain, RenderData &renderData, CommandList &commandList, Resource &input, Resource &output) {
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.transitionBarrier(input, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_FOG);

    const Resource *fogRts[] = {&output};
    commandList.OMSetRenderTargetsNoDepth(fogRts);

    commandList.setSrvInDescriptorTable(0, 0, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 1, input);

    FogCB fogCB;
    fogCB.screenWidth = static_cast<float>(swapChain.getWidth());
    fogCB.screenHeight = static_cast<float>(swapChain.getHeight());
    fogCB.fogPower = fogPower;
    fogCB.fogColor = XMFLOAT3(fogColor[0], fogColor[1], fogColor[2]);
    fogCB.viewMatrixInverse = camera->getInvViewMatrix();
    fogCB.projMatrixInverse = camera->getInvProjectionMatrix();
    fogCB.cameraPosition = XMFLOAT4(camera->getEyePosition().x, camera->getEyePosition().y, camera->getEyePosition().z, 1);
    commandList.setRoot32BitConstant(1, fogCB);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);
}

void SceneImpl::renderDof(SwapChain &swapChain, RenderData &renderData, CommandList &commandList, Resource &input, Resource &output) {
    // DOF
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.transitionBarrier(input, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getDofMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_DOF_BLUR);

    const Resource *dofBlurRts[] = {&renderData.getDofMap()};
    commandList.OMSetRenderTargetsNoDepth(dofBlurRts);

    commandList.setSrvInDescriptorTable(0, 0, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 1, input);

    DofCB dofCB;
    dofCB.screenWidth = static_cast<float>(swapChain.getWidth());
    dofCB.screenHeight = static_cast<float>(swapChain.getHeight());
    commandList.setRoot32BitConstant(1, dofCB);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);

    // DOF 2
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.transitionBarrier(renderData.getDofMap(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_DOF);

    const Resource *dofRts[] = {&output};
    commandList.OMSetRenderTargetsNoDepth(dofRts);

    commandList.setSrvInDescriptorTable(0, 0, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 1, renderData.getDofMap());

    commandList.setRoot32BitConstant(1, dofCB);

    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    commandList.draw(6u);
}

void SceneImpl::renderPostProcesses(RenderData &renderData, std::vector<PostProcessImpl *> &postProcesses, CommandList &commandList,
                                    AlternatingResources &alternatingResources, size_t enabledPostProcessesCount,
                                    float screenWidth, float screenHeight) {
    commandList.RSSetViewport(0.f, 0.f, screenWidth, screenHeight);
    commandList.IASetPrimitiveTopologyTriangleList();

    size_t postProcessIndex = 0u;
    Resource *source{}, *destination{};
    for (PostProcessImpl *postProcess : postProcesses) {
        if (!postProcess->isEnabled()) {
            continue;
        }

        // Render
        renderPostProcess(renderData, *postProcess, commandList, nullptr, alternatingResources, screenWidth, screenHeight);
        alternatingResources.swapResources();
        postProcessIndex++;
    }
}

void SceneImpl::renderBloom(SwapChain &swapChain, RenderData &renderData, CommandList &commandList,
                            Resource &input, Resource &output) {
    // Blur the bloom map
    const float screenWidth = static_cast<float>(swapChain.getWidth());
    const float screenHeight = static_cast<float>(swapChain.getHeight());
    AlternatingResources &alternatingResourcesForBlur = renderData.getHelperAlternatingResources();
    renderPostProcess(renderData, renderData.getPostProcessForBloom(), commandList,
                      &input, alternatingResourcesForBlur, screenWidth, screenHeight);

    // Apply bloom on output buffer
    Resource &blurredBloomMap = alternatingResourcesForBlur.getDestination();
    commandList.transitionBarrier(blurredBloomMap, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_APPLY_BLOOM);
    PostProcessApplyBloomCB cb = {};
    cb.screenWidth = static_cast<float>(swapChain.getWidth());
    cb.screenHeight = static_cast<float>(swapChain.getHeight());
    commandList.setRoot32BitConstant(0, cb);
    commandList.setSrvInDescriptorTable(1, 0, blurredBloomMap);
    commandList.OMSetRenderTargetNoDepth(output);
    commandList.IASetVertexBuffer(renderData.getFullscreenVB());
    commandList.draw(6u);
}

void SceneImpl::renderPostProcess(RenderData &renderData, PostProcessImpl &postProcess, CommandList &commandList, Resource *optionalInput,
                                  AlternatingResources &alternatingResources, float screenWidth, float screenHeight) {
    assert(postProcess.isEnabled());
    Resource *source{}, *destination{};

    // Render post process base on its type
    if (postProcess.getType() == PostProcessImpl::Type::CONVOLUTION) {
        getSourceAndDestinationForPostProcess(alternatingResources, optionalInput, source, destination);
        prepareSourceAndDestinationForPostProcess(commandList, *source, *destination, false);

        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().convolution;
        postProcessData.screenWidth = screenWidth;
        postProcessData.screenHeight = screenHeight;

        // Set state
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_CONVOLUTION);
        commandList.setRoot32BitConstant(0, postProcessData);
        commandList.setSrvInDescriptorTable(1, 0, *source);
        commandList.OMSetRenderTargetNoDepth(*destination);
        commandList.IASetVertexBuffer(renderData.getFullscreenVB());

        // Render
        commandList.draw(6u);
    } else if (postProcess.getType() == PostProcessImpl::Type::BLACK_BARS) {
        getSourceAndDestinationForPostProcess(alternatingResources, optionalInput, source, destination);
        prepareSourceAndDestinationForPostProcess(commandList, *source, *destination, false);

        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().blackBars;
        postProcessData.screenWidth = screenWidth;
        postProcessData.screenHeight = screenHeight;

        // Set state
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_BLACK_BARS);
        commandList.setRoot32BitConstant(0, postProcessData);
        commandList.setSrvInDescriptorTable(1, 0, *source);
        commandList.OMSetRenderTargetNoDepth(*destination);
        commandList.IASetVertexBuffer(renderData.getFullscreenVB());

        // Render
        commandList.draw(6u);
    } else if (postProcess.getType() == PostProcessImpl::Type::LINEAR_COLOR_CORRECTION) {
        getSourceAndDestinationForPostProcess(alternatingResources, optionalInput, source, destination);
        prepareSourceAndDestinationForPostProcess(commandList, *source, *destination, false);

        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().linearColorCorrection;
        postProcessData.screenWidth = screenWidth;
        postProcessData.screenHeight = screenHeight;

        // Set state
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_LINEAR_COLOR_CORRECTION);
        commandList.setRoot32BitConstant(0, postProcessData);
        commandList.setSrvInDescriptorTable(1, 0, *source);
        commandList.OMSetRenderTargetNoDepth(*destination);
        commandList.IASetVertexBuffer(renderData.getFullscreenVB());

        // Render
        commandList.draw(6u);
    } else if (postProcess.getType() == PostProcessImpl::Type::GAUSSIAN_BLUR) {
        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().gaussianBlur;
        postProcessData.cb.screenWidth = screenWidth;
        postProcessData.cb.screenHeight = screenHeight;

        const UINT threadGroupCountX = static_cast<UINT>(screenWidth / 16 + 0.5f);
        const UINT threadGroupCountY = static_cast<UINT>(screenHeight / 16 + 0.5f);

        // Set cross-pass state
        commandList.IASetVertexBuffer(renderData.getFullscreenVB());

        // Render all passes of the blur filter
        for (auto passIndex = 0u; passIndex < postProcessData.passCount; passIndex++) {
            // Render horizontal pass
            getSourceAndDestinationForPostProcess(alternatingResources, optionalInput, source, destination);
            prepareSourceAndDestinationForPostProcess(commandList, *source, *destination, true);
            commandList.setPipelineStateAndComputeRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR_COMPUTE_HORIZONTAL);
            commandList.setSrvInDescriptorTable(0, 0, *source);
            commandList.setUavInDescriptorTable(0, 1, *destination);
            commandList.setRoot32BitConstant(1, postProcessData.cb);
            commandList.dispatch(threadGroupCountX, threadGroupCountY, 1u);
            alternatingResources.swapResources();

            // Clear optional input so rest of the passes are done in ping-pong manner
            optionalInput = nullptr;

            // Render vertical pass
            getSourceAndDestinationForPostProcess(alternatingResources, optionalInput, source, destination);
            prepareSourceAndDestinationForPostProcess(commandList, *source, *destination, true);
            commandList.setPipelineStateAndComputeRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR_COMPUTE_VERTICAL);
            commandList.setSrvInDescriptorTable(0, 0, *source);
            commandList.setUavInDescriptorTable(0, 1, *destination);
            commandList.setRoot32BitConstant(1, postProcessData.cb);
            commandList.dispatch(threadGroupCountX, threadGroupCountY, 1u);

            const bool lastPass = (passIndex == postProcessData.passCount - 1);
            if (!lastPass) {
                alternatingResources.swapResources();
            }
        }
    } else if (postProcess.getType() == PostProcessImpl::Type::FXAA) {
        getSourceAndDestinationForPostProcess(alternatingResources, optionalInput, source, destination);
        prepareSourceAndDestinationForPostProcess(commandList, *source, *destination, false);

        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().fxaa;
        postProcessData.screenWidth = screenWidth;
        postProcessData.screenHeight = screenHeight;

        // Set state
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_FXAA);
        commandList.setRoot32BitConstant(0, postProcessData);
        commandList.setSrvInDescriptorTable(1, 0, *source);
        commandList.OMSetRenderTargetNoDepth(*destination);
        commandList.IASetVertexBuffer(renderData.getFullscreenVB());

        // Render
        commandList.draw(6u);
    } else {
        UNREACHABLE_CODE();
    }
}

void SceneImpl::renderD2DTexts(SwapChain &swapChain) {
    // Get the contexts
    auto &d2dDeviceContext = ApplicationImpl::getInstance().getD2DContext().getD2DDeviceContext();
    auto &d3d11DeviceContext = ApplicationImpl::getInstance().getD2DContext().getD3D11DeviceContext();

    // Acquire D2D back buffer. Acquired buffer is automatically released by the destructor and flush is made
    AcquiredD2DWrappedResource backBuffer = swapChain.getCurrentD2DWrappedBackBuffer().acquire();
    auto &d2dBackBuffer = backBuffer.getD2DResource();

    // Render text directly to the back buffer.
    const D2D1_SIZE_F rtSize = d2dBackBuffer->GetSize();
    const D2D1_RECT_F textRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);
    d2dDeviceContext->SetTarget(d2dBackBuffer.Get());
    d2dDeviceContext->BeginDraw();
    d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
    for (auto text : texts) {
        text->update();
        text->draw(textRect);
    }
    throwIfFailed(d2dDeviceContext->EndDraw());
}

void SceneImpl::renderSprite(SwapChain &swapChain, CommandList &commandList, SpriteImpl *sprite, VertexBuffer &fullscreenVB) {
    auto &tex = sprite->getTextureImpl();
    auto &backBuffer = swapChain.getCurrentBackBuffer();

    // Prepare constant buffer
    SpriteCB spriteData = sprite->getData();
    spriteData.screenWidth = static_cast<float>(swapChain.getWidth());
    spriteData.screenHeight = static_cast<float>(swapChain.getHeight());

    // Set state
    commandList.transitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SPRITE);
    commandList.setRoot32BitConstant(0, spriteData);
    commandList.setSrvInDescriptorTable(1, 0, tex);
    commandList.OMSetRenderTargetNoDepth(backBuffer);
    commandList.IASetVertexBuffer(fullscreenVB);

    // Render
    commandList.draw(6u);
}

void SceneImpl::render(SwapChain &swapChain, RenderData &renderData) {
    auto &application = ApplicationImpl::getInstance();
    auto &commandQueue = application.getDirectCommandQueue();
    auto &backBuffer = swapChain.getCurrentBackBuffer();
    auto &alternatingResources = renderData.getSceneAlternatingResources();
    application.flushAllResources();
    inspectObjectsNotReady();

    // Render shadow maps
    if (ApplicationImpl::getInstance().getSettingsImpl().getShadowsQuality() > 0) {
        CommandList commandListShadowMap{commandQueue};
        SET_OBJECT_NAME(commandListShadowMap, L"cmdListShadowMap")
        renderShadowMaps(swapChain, renderData, commandListShadowMap);
        commandListShadowMap.close();
        commandQueue.executeCommandListAndSignal(commandListShadowMap);
    }

    // GBuffer
    CommandList commandListGBuffer{commandQueue};
    SET_OBJECT_NAME(commandListGBuffer, L"cmdListGBuffer");
    renderGBuffer(swapChain, renderData, commandListGBuffer);
    commandListGBuffer.close();
    commandQueue.executeCommandListAndSignal(commandListGBuffer);

    // SSAO
    if (ApplicationImpl::getInstance().getSettings().getSsaoEnabled()) {
        CommandList commandListSSAO{commandQueue};
        SET_OBJECT_NAME(commandListSSAO, L"cmdListSSAO");
        renderSSAO(swapChain, renderData, commandListSSAO);
        commandListSSAO.close();
        commandQueue.executeCommandListAndSignal(commandListSSAO);
    }

    // Command list for fixed post processes - 2D operations
    CommandList commandListFixedPostProcesses{commandQueue};
    SET_OBJECT_NAME(commandListFixedPostProcesses, L"cmdListFixedPostProcesses");

    // Lighting - merge GBuffers and apply per-pixel shading
    renderLighting(swapChain, renderData, commandListFixedPostProcesses, alternatingResources.getDestination());
    alternatingResources.swapResources();

    // SSR
    if (ApplicationImpl::getInstance().getSettings().getSsrEnabled()) {
        Resource &source = alternatingResources.getSource();
        Resource &destination = alternatingResources.getDestination();
        renderSSRandMerge(swapChain, renderData, commandListFixedPostProcesses, source, destination);
        alternatingResources.swapResources();
    }

    // Fog
    if (ApplicationImpl::getInstance().getSettings().getFogEnabled()) {
        Resource &source = alternatingResources.getSource();
        Resource &destination = alternatingResources.getDestination();
        renderFog(swapChain, renderData, commandListFixedPostProcesses, source, destination);
        alternatingResources.swapResources();
    }

    // Dof
    if (ApplicationImpl::getInstance().getSettings().getDofEnabled()) {
        Resource &source = alternatingResources.getSource();
        Resource &destination = alternatingResources.getDestination();
        renderDof(swapChain, renderData, commandListFixedPostProcesses, source, destination);
        alternatingResources.swapResources();
    }

    // Send fixed post processes command lsit
    commandListFixedPostProcesses.close();
    commandQueue.executeCommandListAndSignal(commandListFixedPostProcesses);

    // Render post processes
    CommandList commandListPostProcess{commandQueue};
    SET_OBJECT_NAME(commandListPostProcess, L"cmdListPostProcess");
    commandListPostProcess.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    commandListPostProcess.RSSetScissorRectNoScissor();
    commandListPostProcess.IASetPrimitiveTopologyTriangleList();
    const auto postProcessesCount = getEnabledPostProcessesCount();
    const bool shouldRenderPostProcesses = (postProcessesCount > 0);
    if (shouldRenderPostProcesses) {
        renderPostProcesses(renderData, postProcesses, commandListPostProcess, alternatingResources, postProcessesCount,
                            static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    }

    // Bloom
    const bool bloomEnabled = true; //  TODO
    if (bloomEnabled) {
        Resource &source = renderData.getBloomMap();
        Resource &destination = alternatingResources.getSource(); // render to previously rendered frame
        renderBloom(swapChain, renderData, commandListPostProcess, source, destination);
    }

    // Copy to back buffers
    commandListPostProcess.transitionBarrier(alternatingResources.getSource(), D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandListPostProcess.transitionBarrier(backBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
    commandListPostProcess.copyResource(backBuffer, alternatingResources.getSource());

    // Close command list and submit it to the GPU
    commandListPostProcess.close();
    commandQueue.executeCommandListAndSignal(commandListPostProcess);

    CommandList commandListSprite{commandQueue};
    commandListSprite.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    commandListSprite.RSSetScissorRectNoScissor();
    commandListSprite.IASetPrimitiveTopologyTriangleList();
    for (auto &sprite : sprites)
        renderSprite(swapChain, commandListSprite, sprite, renderData.getFullscreenVB());

    // If there are no D2D content to render, there will be no implicit transition to present, hence the manual barrier

    if (texts.empty()) {
        commandListSprite.transitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
    } else {
        commandListSprite.transitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    commandListSprite.close();
    const uint64_t fenceValue = commandQueue.executeCommandListAndSignal(commandListSprite);

    // D2D, additional command list is inserted
    if (!texts.empty()) {
        renderD2DTexts(swapChain);
    }

    // Present (swap back buffers) and wait for next frame's fence
    swapChain.present(fenceValue);
    commandQueue.waitOnCpu(swapChain.getFenceValueForCurrentBackBuffer());
}

/// QUERY used in perf. measurement.
///UINT64 *queryData = nullptr;
///queryResult->getResource().Get()->Map(0, nullptr, (void **)&queryData);
///
///static int fpsTab[100];
///static int fpsTabIter;
///fpsTabIter++;
///
///fpsTab[fpsTabIter % 100] = int(queryData[1] - queryData[0]);
///
///int fpsSum = 0;
///for (int i = 0; i < 100; i++) {
///    fpsSum += fpsTab[i];
///}
///
///DXD::log("Time: %d\n", fpsSum/100);
///queryResult->getResource().Get()->Unmap(0, nullptr);
