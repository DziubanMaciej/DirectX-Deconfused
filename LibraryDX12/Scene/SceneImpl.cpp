#include "SceneImpl.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "CommandList/CommandQueue.h"
#include "Resource/ConstantBuffers.h"
#include "Scene/RenderData.h"
#include "Utility/DxObjectNaming.h"
#include "Utility/ThrowIfFailed.h"
#include "Window/WindowImpl.h"

#include <ExternalHeaders/Wrappers/d3dx12.h>
#include <algorithm>
#include <cassert>

// --------------------------------------------------------------------------- API

namespace DXD {
std::unique_ptr<Scene> Scene::create(DXD::Application &application) {
    return std::unique_ptr<Scene>{new SceneImpl(*static_cast<ApplicationImpl *>(&application))};
}
} // namespace DXD

SceneImpl::SceneImpl(ApplicationImpl &application)
    : application(application),
      lightConstantBuffer(application.getDevice(), application.getDescriptorController(), sizeof(LightingConstantBuffer)) {

    ID3D12DevicePtr device = application.getDevice();
    auto &commandQueue = application.getDirectCommandQueue();

    // Record command list for GPU upload
    CommandList commandList{commandQueue};
    postProcessVB = std::make_unique<VertexBuffer>(device, commandList, postProcessSquare, 6, 12);
    SET_OBJECT_NAME(*postProcessVB, L"PostProcessVB");
    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    const uint64_t fenceValue = commandQueue.executeCommandListAndSignal(commandList);
}

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

void SceneImpl::addLight(DXD::Light &light) {
    lights.push_back(static_cast<LightImpl *>(&light));
}

void SceneImpl::addText(DXD::Text &text) {
    texts.push_back(static_cast<TextImpl *>(&text));
}

void SceneImpl::addPostProcess(DXD::PostProcess &postProcess) {
    postProcesses.push_back(static_cast<PostProcessImpl *>(&postProcess));
}

bool SceneImpl::removeLight(DXD::Light &light) {
    for (auto it = lights.begin(); it != lights.end(); it++) {
        if (*it == &light) {
            lights.erase(it);
            return 1;
        }
    }
    return 0;
}

void SceneImpl::addObject(DXD::Object &object) {
    objectsNotReady.insert(static_cast<ObjectImpl *>(&object));
}

bool SceneImpl::removeObject(DXD::Object &object) {
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

void SceneImpl::getAndPrepareSourceAndDestinationForPostProcess(CommandList &commandList,
                                                                PostProcessRenderTargets &renderTargets,
                                                                bool first, bool last,
                                                                Resource *initialInput, Resource *finalOutput,
                                                                Resource *&outSource, Resource *&outDestination, bool compute) {
    // Get resources
    outSource = (first && initialInput) ? initialInput : &renderTargets.getSource();
    outDestination = (last && finalOutput) ? finalOutput : &renderTargets.getDestination();

    // Set states
    if (compute) {
        commandList.transitionBarrier(*outSource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        commandList.transitionBarrier(*outDestination, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    } else {
        commandList.transitionBarrier(*outSource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        commandList.transitionBarrier(*outDestination, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
}

// --------------------------------------------------------------------------- Render methods

const static FLOAT blackColor[] = {0.f, 0.f, 0.f, 1.f};

void SceneImpl::renderShadowMaps(SwapChain &swapChain, RenderData &renderData, CommandList &commandList) {
    const auto shadowMapsUsed = std::min(size_t{8u}, lights.size());

    for (int i = 0; i < shadowMapsUsed; i++) {
        commandList.transitionBarrier(renderData.getShadowMap(i), D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }

    commandList.RSSetViewport(0.f, 0.f, 2048.f, 2048.f);
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    int lightIdx = 0;

    for (LightImpl *light : lights) {
        commandList.OMSetRenderTargetDepthOnly(renderData.getShadowMap(lightIdx));
        commandList.clearDepthStencilView(renderData.getShadowMap(lightIdx), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

        // View projection matrix
        camera->setAspectRatio(1.0f);

        const auto lightPosition = XMLoadFloat3(&light->getPosition());
        const auto lightDirection = XMLoadFloat3(&light->getDirection());
        const auto lightFocusPoint = XMVectorAdd(lightDirection, lightPosition);
        const XMMATRIX smViewMatrix = XMMatrixLookAtLH(lightPosition, lightFocusPoint, XMVectorSet(0, 1, 0, 0.f));
        if (light->getType() == DXD::LightType::SPOT_LIGHT) {
            const XMMATRIX smProjectionMatrix = XMMatrixPerspectiveFovLH(90, 1, 0.1f, 160.0f);
            light->smViewProjectionMatrix = XMMatrixMultiply(smViewMatrix, smProjectionMatrix);
        } else {
            const XMMATRIX smProjectionMatrix = XMMatrixOrthographicLH(40, 40, 0.1f, 160.0f);
            light->smViewProjectionMatrix = XMMatrixMultiply(smViewMatrix, smProjectionMatrix);
        }

        // Draw NORMAL
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SM_NORMAL);
        for (ObjectImpl *object : objects) {
            MeshImpl &mesh = object->getMesh();
            if (mesh.getShadowMapPipelineStateIdentifier() == commandList.getPipelineStateIdentifier()) {
                SMmvp smmvp;
                smmvp.modelViewProjectionMatrix = XMMatrixMultiply(object->getModelMatrix(), light->smViewProjectionMatrix);
                commandList.setRoot32BitConstant(0, smmvp);

                commandList.IASetVertexAndIndexBuffer(mesh);
                commandList.draw(static_cast<UINT>(mesh.getVerticesCount()));
            }
        }

        // Draw TEXTURE NORMAL
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SM_TEXTURE_NORMAL);
        for (ObjectImpl *object : objects) {
            MeshImpl &mesh = object->getMesh();
            if (mesh.getShadowMapPipelineStateIdentifier() == commandList.getPipelineStateIdentifier()) {
                SMmvp smmvp;
                smmvp.modelViewProjectionMatrix = XMMatrixMultiply(object->getModelMatrix(), light->smViewProjectionMatrix);
                commandList.setRoot32BitConstant(0, smmvp);

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

void SceneImpl::renderGBuffer(SwapChain &swapChain, RenderData &renderData, CommandList &commandList, VertexBuffer &fullscreenVB) {
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
    float aspectRatio = (float)swapChain.getWidth() / swapChain.getHeight();
    camera->setAspectRatio(aspectRatio);
    XMMATRIX viewMatrix = camera->getViewMatrix();
    XMMATRIX projectionMatrix = camera->getProjectionMatrix();
    XMMATRIX vpMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

    // LightingConstantBuffer
    auto lightCb = lightConstantBuffer.getData<LightingConstantBuffer>();
    lightCb->cameraPosition = XMFLOAT4(camera->getEyePosition().x, camera->getEyePosition().y, camera->getEyePosition().z, 1);
    lightCb->lightsSize = 0;
    lightCb->ambientLight = XMFLOAT3(ambientLight[0], ambientLight[1], ambientLight[2]);
    lightCb->screenWidth = static_cast<float>(swapChain.getWidth());
    lightCb->screenHeight = static_cast<float>(swapChain.getHeight());
    for (LightImpl *light : lights) {
        lightCb->lightColor[lightCb->lightsSize] = XMFLOAT4(light->getColor().x, light->getColor().y, light->getColor().z, light->getPower());
        lightCb->lightPosition[lightCb->lightsSize] = XMFLOAT4(light->getPosition().x, light->getPosition().y, light->getPosition().z, 0);
        lightCb->lightDirection[lightCb->lightsSize] = XMFLOAT4(light->getDirection().x, light->getDirection().y, light->getDirection().z, 0);
        lightCb->smViewProjectionMatrix[lightCb->lightsSize] = light->smViewProjectionMatrix;
        lightCb->lightsSize++;
        if (lightCb->lightsSize >= 8) {
            break;
        }
    }
    lightConstantBuffer.upload();

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

            ObjectProperties op = {};
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

            ObjectProperties op = {};
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

            ObjectProperties op = {};
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

void SceneImpl::renderSSAO(SwapChain &swapChain, RenderData &renderData, CommandList &commandList, VertexBuffer &fullscreenVB) {
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.transitionBarrier(renderData.getSsaoMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SSAO);

    const Resource *ssaoRts[] = {&renderData.getSsaoMap()};
    commandList.OMSetRenderTargetsNoDepth(ssaoRts);

    commandList.setSrvInDescriptorTable(0, 0, renderData.getGBufferNormal());
    commandList.setSrvInDescriptorTable(0, 1, renderData.getDepthStencilBuffer());

    SsaoCB ssaoCB;
    ssaoCB.screenWidth = static_cast<float>(swapChain.getWidth());
    ssaoCB.screenHeight = static_cast<float>(swapChain.getHeight());
    ssaoCB.viewMatrixInverse = camera->getInvViewMatrix();
    ssaoCB.projMatrixInverse = camera->getInvProjectionMatrix();
    commandList.setRoot32BitConstant(1, ssaoCB);

    commandList.IASetVertexBuffer(fullscreenVB);

    commandList.draw(6u);

    commandList.transitionBarrier(renderData.getSsaoMap(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void SceneImpl::renderLighting(SwapChain &swapChain, RenderData &renderData, CommandList &commandList, Resource &output, VertexBuffer &fullscreenVB) {

    // SSR
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.transitionBarrier(renderData.getSsrMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SSR);

    const Resource *ssrRts[] = {&renderData.getSsrMap()};
    commandList.OMSetRenderTargetsNoDepth(ssrRts);

    commandList.clearRenderTargetView(renderData.getSsrMap(), backgroundColor);

    commandList.setSrvInDescriptorTable(0, 0, renderData.getGBufferNormal());
    commandList.setSrvInDescriptorTable(0, 1, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 2, renderData.getGBufferSpecular());
    commandList.setSrvInDescriptorTable(0, 3, renderData.getLightingOutput());

    SsrCB ssrCB;
    ssrCB.cameraPosition = XMFLOAT4(camera->getEyePosition().x, camera->getEyePosition().y, camera->getEyePosition().z, 1);
    ssrCB.screenWidth = static_cast<float>(swapChain.getWidth());
    ssrCB.screenHeight = static_cast<float>(swapChain.getHeight());
    ssrCB.viewMatrixInverse = camera->getInvViewMatrix();
    ssrCB.projMatrixInverse = camera->getInvProjectionMatrix();
    ssrCB.viewProjectionMatrix = XMMatrixMultiply(camera->getViewMatrix(), camera->getProjectionMatrix());
    commandList.setRoot32BitConstant(1, ssrCB);

    commandList.IASetVertexBuffer(fullscreenVB);

    commandList.draw(6u);

    commandList.transitionBarrier(renderData.getSsrMap(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // Lighting
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_LIGHTING);

    commandList.transitionBarrier(renderData.getLightingOutput(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.transitionBarrier(renderData.getBloomMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.clearRenderTargetView(renderData.getBloomMap(), blackColor);
    commandList.clearRenderTargetView(renderData.getLightingOutput(), backgroundColor);
    commandList.clearRenderTargetView(output, backgroundColor);

    const Resource *lightingRts[] = {&output, &renderData.getBloomMap(), &renderData.getLightingOutput()};
    commandList.OMSetRenderTargetsNoDepth(lightingRts);

    commandList.setCbvInDescriptorTable(0, 0, lightConstantBuffer);
    commandList.setSrvInDescriptorTable(0, 1, renderData.getGBufferAlbedo());
    commandList.setSrvInDescriptorTable(0, 2, renderData.getGBufferNormal());
    commandList.setSrvInDescriptorTable(0, 3, renderData.getGBufferSpecular());
    commandList.setSrvInDescriptorTable(0, 4, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 5, renderData.getSsaoMap());
    commandList.setSrvInDescriptorTable(0, 6, renderData.getSsrMap());
    for (auto shadowMapIndex = 0u; shadowMapIndex < 8; shadowMapIndex++) {
        commandList.setSrvInDescriptorTable(0, shadowMapIndex + 7, renderData.getShadowMap(shadowMapIndex));
    }

    InverseViewProj invVP;
    invVP.viewMatrixInverse = camera->getInvViewMatrix();
    invVP.projMatrixInverse = camera->getInvProjectionMatrix();
    commandList.setRoot32BitConstant(1, invVP);

    commandList.IASetVertexBuffer(fullscreenVB);

    commandList.draw(6u);

    commandList.transitionBarrier(renderData.getLightingOutput(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void SceneImpl::renderPostProcesses(std::vector<PostProcessImpl *> &postProcesses, CommandList &commandList, VertexBuffer &fullscreenVB,
                                    Resource *input, PostProcessRenderTargets &renderTargets, Resource *output,
                                    size_t enabledPostProcessesCount, float screenWidth, float screenHeight) {
    commandList.RSSetViewport(0.f, 0.f, screenWidth, screenHeight);
    commandList.IASetPrimitiveTopologyTriangleList();

    size_t postProcessIndex = 0u;
    Resource *source{}, *destination{};
    for (PostProcessImpl *postProcess : postProcesses) {
        if (!postProcess->isEnabled()) {
            continue;
        }

        // Select input and output buffer for current post process
        const bool firstPostProcess = (postProcessIndex == 0);
        const bool lastPostProcess = (postProcessIndex == enabledPostProcessesCount - 1);
        Resource *postProcessInput = firstPostProcess ? input : nullptr;
        Resource *postProcessOutput = lastPostProcess ? output : nullptr;

        // Render
        renderPostProcess(*postProcess, commandList, fullscreenVB, postProcessInput, renderTargets, postProcessOutput, screenWidth, screenHeight);
        renderTargets.swapResources();
        postProcessIndex++;
    }
}

void SceneImpl::renderBloom(SwapChain &swapChain, CommandList &commandList, PostProcessImpl &bloomBlurEffect, VertexBuffer &fullscreenVB,
                            Resource &bloomMap, PostProcessRenderTargets &renderTargets, Resource &output) {
    // Blur the bloom map
    renderPostProcess(bloomBlurEffect, commandList, fullscreenVB,
                      &bloomMap, renderTargets, &bloomMap,
                      static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));

    // Apply bloom on output buffer
    PostProcessApplyBloomCB cb = {};
    cb.screenWidth = static_cast<float>(swapChain.getWidth());
    cb.screenHeight = static_cast<float>(swapChain.getHeight());
    commandList.transitionBarrier(bloomMap, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_APPLY_BLOOM);
    commandList.setRoot32BitConstant(0, cb);
    commandList.setSrvInDescriptorTable(1, 0, bloomMap);
    commandList.OMSetRenderTargetNoDepth(output);
    commandList.IASetVertexBuffer(fullscreenVB);
    commandList.draw(6u);
}

void SceneImpl::renderPostProcess(PostProcessImpl &postProcess, CommandList &commandList, VertexBuffer &fullscreenVB,
                                  Resource *input, PostProcessRenderTargets &renderTargets, Resource *output,
                                  float screenWidth, float screenHeight) {
    assert(postProcess.isEnabled());
    Resource *source{}, *destination{};

    // Render post process base on its type
    if (postProcess.getType() == PostProcessImpl::Type::CONVOLUTION) {
        getAndPrepareSourceAndDestinationForPostProcess(commandList, renderTargets, true, true, input, output, source, destination);

        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().convolution;
        postProcessData.screenWidth = screenWidth;
        postProcessData.screenHeight = screenHeight;

        // Set state
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_CONVOLUTION);
        commandList.setRoot32BitConstant(0, postProcessData);
        commandList.setSrvInDescriptorTable(1, 0, *source);
        commandList.OMSetRenderTargetNoDepth(*destination);
        commandList.IASetVertexBuffer(fullscreenVB);

        // Render
        commandList.draw(6u);
    } else if (postProcess.getType() == PostProcessImpl::Type::BLACK_BARS) {
        getAndPrepareSourceAndDestinationForPostProcess(commandList, renderTargets, true, true, input, output, source, destination);

        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().blackBars;
        postProcessData.screenWidth = screenWidth;
        postProcessData.screenHeight = screenHeight;

        // Set state
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_BLACK_BARS);
        commandList.setRoot32BitConstant(0, postProcessData);
        commandList.setSrvInDescriptorTable(1, 0, *source);
        commandList.OMSetRenderTargetNoDepth(*destination);
        commandList.IASetVertexBuffer(fullscreenVB);

        // Render
        commandList.draw(6u);
    } else if (postProcess.getType() == PostProcessImpl::Type::LINEAR_COLOR_CORRECTION) {
        getAndPrepareSourceAndDestinationForPostProcess(commandList, renderTargets, true, true, input, output, source, destination);

        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().linearColorCorrection;
        postProcessData.screenWidth = screenWidth;
        postProcessData.screenHeight = screenHeight;

        // Set state
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_LINEAR_COLOR_CORRECTION);
        commandList.setRoot32BitConstant(0, postProcessData);
        commandList.setSrvInDescriptorTable(1, 0, *source);
        commandList.OMSetRenderTargetNoDepth(*destination);
        commandList.IASetVertexBuffer(fullscreenVB);

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
        commandList.IASetVertexBuffer(fullscreenVB);

        // Render all passes of the blur filter
        for (auto passIndex = 0u; passIndex < postProcessData.passCount; passIndex++) {

            // Render horizontal pass
            const bool firstPass = (passIndex == 0);
            getAndPrepareSourceAndDestinationForPostProcess(commandList, renderTargets, firstPass, false, input, nullptr, source, destination, true);
            commandList.setPipelineStateAndComputeRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR_COMPUTE_HORIZONTAL);
            commandList.setSrvInDescriptorTable(0, 0, *source);
            commandList.setUavInDescriptorTable(0, 1, *destination);
            commandList.setRoot32BitConstant(1, postProcessData.cb);
            commandList.dispatch(threadGroupCountX, threadGroupCountY, 1u);

            renderTargets.swapResources();

            // Render vertical pass
            const bool lastPass = (passIndex == postProcessData.passCount - 1);
            getAndPrepareSourceAndDestinationForPostProcess(commandList, renderTargets, false, lastPass, nullptr, output, source, destination, true);
            commandList.setPipelineStateAndComputeRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR_COMPUTE_VERTICAL);
            commandList.setSrvInDescriptorTable(0, 0, *source);
            commandList.setUavInDescriptorTable(0, 1, *destination);
            commandList.setRoot32BitConstant(1, postProcessData.cb);
            commandList.dispatch(threadGroupCountX, threadGroupCountY, 1u);

            if (!lastPass) {
                renderTargets.swapResources();
            }
        }
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
    for (auto txt : texts) {
        txt->updateText();
        // TODO: this call as TextImpl method
        d2dDeviceContext->DrawText(
            txt->getText().c_str(),
            static_cast<UINT>(txt->getText().size()),
            txt->m_textFormat.Get(),
            &textRect,
            txt->m_textBrush.Get());
    }
    throwIfFailed(d2dDeviceContext->EndDraw());
}

void SceneImpl::render(SwapChain &swapChain, RenderData &renderData) {
    auto &commandQueue = application.getDirectCommandQueue();
    auto &backBuffer = swapChain.getCurrentBackBuffer();
    commandQueue.performResourcesDeletion();
    inspectObjectsNotReady();

    // Render shadow maps
    CommandList commandListShadowMap{commandQueue};
    SET_OBJECT_NAME(commandListShadowMap, L"cmdListShadowMap")
    renderShadowMaps(swapChain, renderData, commandListShadowMap);
    commandListShadowMap.close();
    commandQueue.executeCommandListAndSignal(commandListShadowMap);

    // GBuffer
    CommandList commandListGBuffer{commandQueue};
    SET_OBJECT_NAME(commandListGBuffer, L"cmdListGBuffer");
    renderGBuffer(swapChain, renderData, commandListGBuffer, *postProcessVB);
    commandListGBuffer.close();
    commandQueue.executeCommandListAndSignal(commandListGBuffer);

    // SSAO
    CommandList commandListSSAO{commandQueue};
    SET_OBJECT_NAME(commandListSSAO, L"cmdListSSAO");
    renderSSAO(swapChain, renderData, commandListSSAO, *postProcessVB);
    commandListSSAO.close();
    commandQueue.executeCommandListAndSignal(commandListSSAO);

    // SSR
    // TODO

    // Identify output for the next step: if there are post processes, render to intermediate buffer
    const auto postProcessesCount = getEnabledPostProcessesCount();
    const bool shouldRenderPostProcesses = (postProcessesCount > 0);
    auto &renderLightingOutput = shouldRenderPostProcesses ? renderData.getPostProcessRenderTargets().getDestination() : backBuffer;

    // Lighting
    CommandList commandListLighting{commandQueue};
    SET_OBJECT_NAME(commandListLighting, L"cmdListLighting");
    renderLighting(swapChain, renderData, commandListLighting, renderLightingOutput, *postProcessVB);
    commandListLighting.close();
    commandQueue.executeCommandListAndSignal(commandListLighting);

    // Render post processes
    CommandList commandListPostProcess{commandQueue};
    SET_OBJECT_NAME(commandListPostProcess, L"cmdListPostProcess");
    commandListPostProcess.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    commandListPostProcess.RSSetScissorRectNoScissor();
    commandListPostProcess.IASetPrimitiveTopologyTriangleList();
    if (shouldRenderPostProcesses) {
        renderData.getPostProcessRenderTargets().swapResources();
        renderPostProcesses(postProcesses, commandListPostProcess, *postProcessVB,
                            &renderData.getPostProcessRenderTargets().getSource(), renderData.getPostProcessRenderTargets(), &backBuffer,
                            postProcessesCount, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    }
    renderBloom(swapChain, commandListPostProcess, renderData.getPostProcessForBloom(), *postProcessVB,
                renderData.getBloomMap(), renderData.getPostProcessRenderTargets(), backBuffer);

    // If there are no D2D content to render, there will be no implicit transition to present, hence the manual barrier
    if (texts.empty()) {
        commandListPostProcess.transitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
    }

    // Close command list and submit it to the GPU
    commandListPostProcess.close();
    const uint64_t fenceValue = commandQueue.executeCommandListAndSignal(commandListPostProcess);

    // D2D, additional command list is inserted
    if (!texts.empty()) {
        renderD2DTexts(swapChain);
    }

    // Present (swap back buffers) and wait for next frame's fence
    swapChain.present(fenceValue);
    commandQueue.getFence().waitOnCpu(swapChain.getFenceValueForCurrentBackBuffer());
}
