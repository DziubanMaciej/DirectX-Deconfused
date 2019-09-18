#include "SceneImpl.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "CommandList/CommandQueue.h"
#include "Resource/ConstantBuffers.h"
#include "Scene/RenderData.h"
#include "Utility/DxObjectNaming.h"
#include "Utility/ThrowIfFailed.h"
#include "Window/WindowImpl.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"
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
    CommandList commandList{application.getDescriptorController(), commandQueue.getCommandAllocatorController(), nullptr};
    postProcessVB = std::make_unique<VertexBuffer>(device, commandList, postProcessSquare, 6, 12);
    SET_OBJECT_NAME(*postProcessVB, L"PostProcessVB");
    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    std::vector<CommandList *> commandLists{&commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);
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
        if (!object->isUploadInProgress()) {
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
                                                                Resource &initialInput, Resource &finalOutput,
                                                                Resource *&outSource, Resource *&outDestination) {
    // Get resources
    outSource = first ? &initialInput : &renderTargets.getSource();
    outDestination = last ? &finalOutput : &renderTargets.getDestination();

    // Set states
    commandList.transitionBarrier(*outSource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(*outDestination, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

// --------------------------------------------------------------------------- Render methods

const static FLOAT blackColor[] = {0.f, 0.f, 0.f, 1.f};

void SceneImpl::renderShadowMaps(SwapChain &swapChain, RenderData &renderData, CommandList &commandList) {
    const auto shadowMapsUsed = std::min(size_t{8u}, lights.size());

    for (int i = 0; i < shadowMapsUsed; i++) {
        commandList.transitionBarrier(renderData.getShadowMap(i), D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }

    commandList.RSSetViewport(0.f, 0.f, 2048.f, 2048.f);

    int lightIdx = 0;

    for (LightImpl *light : lights) {
        commandList.OMSetRenderTargetDepthOnly(renderData.getShadowMap(lightIdx));
        commandList.clearDepthStencilView(renderData.getShadowMap(lightIdx).getDsv(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

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
                commandList.setGraphicsRoot32BitConstant(0, smmvp);

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
                commandList.setGraphicsRoot32BitConstant(0, smmvp);

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

void SceneImpl::renderDeferred(SwapChain &swapChain, RenderData &renderData, CommandList &commandList, Resource &output, VertexBuffer &fullscreenVB) {
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));

    // Transition to RENDER_TARGET
    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.transitionBarrier(renderData.getBloomMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.transitionBarrier(renderData.getGBufferAlbedo(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.transitionBarrier(renderData.getGBufferNormal(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.transitionBarrier(renderData.getGBufferSpecular(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.transitionBarrier(renderData.getDepthStencilBuffer(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

    //commandList.OMSetRenderTarget(output, renderData.getDepthStencilBuffer());

    // Render (clear color)
    commandList.clearRenderTargetView(output.getRtv(), backgroundColor);
    commandList.clearRenderTargetView(renderData.getBloomMap().getRtv(), blackColor);
    //commandList.clearRenderTargetView(renderData.getGBufferAlbedo().getRtv(), blackColor);
    //commandList.clearRenderTargetView(renderData.getGBufferNormal().getRtv(), blackColor);
    //commandList.clearRenderTargetView(renderData.getGBufferSpecular().getRtv(), blackColor);
    commandList.clearDepthStencilView(renderData.getDepthStencilBuffer().getDsv(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

    // View projection matrix
    float aspectRatio = (float)swapChain.getWidth() / swapChain.getHeight();
    camera->setAspectRatio(aspectRatio);
    XMMATRIX viewMatrix = camera->getViewMatrix();
    XMMATRIX projectionMatrix = camera->getProjectionMatrix();
    XMMATRIX vpMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

    XMMATRIX viewMatrixInverse = XMMatrixInverse(nullptr, viewMatrix);
    XMMATRIX projMatrixInverse = XMMatrixInverse(nullptr, projectionMatrix);

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

    // Draw DEFAULT
    /*commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_DEFAULT);
    commandList.setCbvInDescriptorTable(2, 0, lightConstantBuffer);
    const Resource *rts[2] = {&output, &renderData.getBloomMap()};
    commandList.OMSetRenderTargets(rts, renderData.getDepthStencilBuffer());
    for (ObjectImpl *object : objects) {
        MeshImpl &mesh = object->getMesh();
        if (mesh.getPipelineStateIdentifier() == commandList.getPipelineStateIdentifier()) {
            ModelMvp mmvp;
            mmvp.modelMatrix = object->getModelMatrix();
            mmvp.modelViewProjectionMatrix = XMMatrixMultiply(mmvp.modelMatrix, vpMatrix);
            commandList.setGraphicsRoot32BitConstant(0, mmvp);

            ObjectProperties op;
            op.objectColor = object->getColor();
            op.objectSpecularity = object->getSpecularity(); //Not supported in objects without normals
            commandList.setGraphicsRoot32BitConstant(1, op);

            commandList.IASetVertexAndIndexBuffer(mesh);
            commandList.drawIndexed(static_cast<UINT>(mesh.getIndicesCount()));
        }
    }*/

    const Resource *rts[] = {&renderData.getGBufferAlbedo(), &renderData.getGBufferNormal(), &renderData.getGBufferSpecular()};
    commandList.OMSetRenderTargets(rts, renderData.getDepthStencilBuffer());

    //Draw NORMAL
    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_NORMAL);
    //commandList.setCbvInDescriptorTable(2, 0, lightConstantBuffer);

    for (ObjectImpl *object : objects) {
        MeshImpl &mesh = object->getMesh();
        if (mesh.getPipelineStateIdentifier() == commandList.getPipelineStateIdentifier()) {
            ModelMvp mmvp;
            mmvp.modelMatrix = object->getModelMatrix();
            mmvp.modelViewProjectionMatrix = XMMatrixMultiply(mmvp.modelMatrix, vpMatrix);
            commandList.setGraphicsRoot32BitConstant(0, mmvp);

            ObjectProperties op;
            op.objectColor = object->getColor();
            op.objectSpecularity = object->getSpecularity();
            commandList.setGraphicsRoot32BitConstant(1, op);

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
            ModelMvp mmvp;
            mmvp.modelMatrix = object->getModelMatrix();
            mmvp.modelViewProjectionMatrix = XMMatrixMultiply(mmvp.modelMatrix, vpMatrix);
            commandList.setGraphicsRoot32BitConstant(0, mmvp);

            ObjectProperties op;
            op.objectColor = object->getColor();
            op.objectSpecularity = object->getSpecularity();
            commandList.setGraphicsRoot32BitConstant(1, op);

            commandList.IASetVertexAndIndexBuffer(mesh);
            commandList.setSrvInDescriptorTable(2, 0, *texture);
            commandList.draw(static_cast<UINT>(mesh.getVerticesCount()));
        }
    }

<<<<<<< HEAD


=======
    // Lighting
>>>>>>> 72a77543e1b5aa14aa8a36db472348f95f6c7bd6
    commandList.transitionBarrier(renderData.getGBufferAlbedo(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getGBufferNormal(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getGBufferSpecular(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(renderData.getDepthStencilBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);


    // SSAO
    commandList.transitionBarrier(renderData.getSsaoMap(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SSAO);

    const Resource *ssaoRts[] = { &renderData.getSsaoMap() };
    commandList.OMSetRenderTargetsNoDepth(ssaoRts);

    commandList.setSrvInDescriptorTable(0, 0, renderData.getGBufferNormal());
    commandList.setSrvInDescriptorTable(0, 1, renderData.getDepthStencilBuffer());

    SsaoCB ssaoCB;
    ssaoCB.screenWidth = static_cast<float>(swapChain.getWidth());
    ssaoCB.screenHeight = static_cast<float>(swapChain.getHeight());
    ssaoCB.viewMatrixInverse = viewMatrixInverse;
    ssaoCB.projMatrixInverse = projMatrixInverse;
    commandList.setGraphicsRoot32BitConstant(1, ssaoCB);

    commandList.IASetVertexBuffer(fullscreenVB);

    commandList.draw(6u);

    commandList.transitionBarrier(renderData.getSsaoMap(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    // Lighting

    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_LIGHTING);

    const Resource *lightingRts[] = {&output, &renderData.getBloomMap()};
    commandList.OMSetRenderTargetsNoDepth(lightingRts);

    commandList.setCbvInDescriptorTable(0, 0, lightConstantBuffer);
    commandList.setSrvInDescriptorTable(0, 1, renderData.getGBufferAlbedo());
    commandList.setSrvInDescriptorTable(0, 2, renderData.getGBufferNormal());
    commandList.setSrvInDescriptorTable(0, 3, renderData.getGBufferSpecular());
    commandList.setSrvInDescriptorTable(0, 4, renderData.getDepthStencilBuffer());
    commandList.setSrvInDescriptorTable(0, 5, renderData.getSsaoMap());
    for (auto shadowMapIndex = 0u; shadowMapIndex < 8; shadowMapIndex++) {
        commandList.setSrvInDescriptorTable(0, shadowMapIndex + 6, renderData.getShadowMap(shadowMapIndex));
    }

    InverseViewProj invVP;
    invVP.viewMatrixInverse = viewMatrixInverse;
    invVP.projMatrixInverse = projMatrixInverse;
    commandList.setGraphicsRoot32BitConstant(1, invVP);

    commandList.IASetVertexBuffer(fullscreenVB);
<<<<<<< HEAD

    commandList.draw(6u);


=======
>>>>>>> 72a77543e1b5aa14aa8a36db472348f95f6c7bd6

    commandList.draw(6u);
}

void SceneImpl::renderPostProcesses(std::vector<PostProcessImpl *> &postProcesses, CommandList &commandList, VertexBuffer &fullscreenVB,
                                    Resource &input, PostProcessRenderTargets &renderTargets, Resource &output,
                                    size_t enabledPostProcessesCount, float screenWidth, float screenHeight) {
    size_t postProcessIndex = 0u;
    Resource *source{}, *destination{};
    for (PostProcessImpl *postProcess : postProcesses) {
        if (!postProcess->isEnabled()) {
            continue;
        }

        // Constants
        const bool firstPostProcess = (postProcessIndex == 0);
        const bool lastPostProcess = (postProcessIndex == enabledPostProcessesCount - 1);

        // Render post process base on its type
        if (postProcess->getType() == PostProcessImpl::Type::CONVOLUTION) {
            getAndPrepareSourceAndDestinationForPostProcess(commandList, renderTargets, firstPostProcess, lastPostProcess, input, output, source, destination);

            // Prepare constant buffer
            auto &postProcessData = postProcess->getData().convolution;
            postProcessData.screenWidth = screenWidth;
            postProcessData.screenHeight = screenHeight;

            // Set state
            commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_CONVOLUTION);
            commandList.setGraphicsRoot32BitConstant(0, postProcessData);
            commandList.setSrvInDescriptorTable(1, 0, *source);
            commandList.OMSetRenderTargetNoDepth(*destination);
            commandList.IASetVertexBuffer(fullscreenVB);

            // Render
            commandList.draw(6u);
        } else if (postProcess->getType() == PostProcessImpl::Type::BLACK_BARS) {
            getAndPrepareSourceAndDestinationForPostProcess(commandList, renderTargets, firstPostProcess, lastPostProcess, input, output, source, destination);

            // Prepare constant buffer
            auto &postProcessData = postProcess->getData().blackBars;
            postProcessData.screenWidth = screenWidth;
            postProcessData.screenHeight = screenHeight;

            // Set state
            commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_BLACK_BARS);
            commandList.setGraphicsRoot32BitConstant(0, postProcessData);
            commandList.setSrvInDescriptorTable(1, 0, *source);
            commandList.OMSetRenderTargetNoDepth(*destination);
            commandList.IASetVertexBuffer(fullscreenVB);

            // Render
            commandList.draw(6u);
        } else if (postProcess->getType() == PostProcessImpl::Type::LINEAR_COLOR_CORRECTION) {
            getAndPrepareSourceAndDestinationForPostProcess(commandList, renderTargets, firstPostProcess, lastPostProcess, input, output, source, destination);

            // Prepare constant buffer
            auto &postProcessData = postProcess->getData().linearColorCorrection;
            postProcessData.screenWidth = screenWidth;
            postProcessData.screenHeight = screenHeight;

            // Set state
            commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_LINEAR_COLOR_CORRECTION);
            commandList.setGraphicsRoot32BitConstant(0, postProcessData);
            commandList.setSrvInDescriptorTable(1, 0, *source);
            commandList.OMSetRenderTargetNoDepth(*destination);
            commandList.IASetVertexBuffer(fullscreenVB);

            // Render
            commandList.draw(6u);
        } else if (postProcess->getType() == PostProcessImpl::Type::GAUSSIAN_BLUR) {
            // Prepare constant buffer
            auto &postProcessData = postProcess->getData().gaussianBlur;
            postProcessData.cb.screenWidth = screenWidth;
            postProcessData.cb.screenHeight = screenHeight;

            // Set cross-pass state
            commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR);
            commandList.IASetVertexBuffer(fullscreenVB);

            // Render all passes of the blur filter
            for (auto passIndex = 0u; passIndex < postProcessData.passCount; passIndex++) {
                // Render horizontal pass
                const bool firstPass = firstPostProcess && (passIndex == 0);
                getAndPrepareSourceAndDestinationForPostProcess(commandList, renderTargets, firstPass, false, input, output, source, destination);
                postProcessData.cb.horizontal = true;
                commandList.OMSetRenderTargetNoDepth(*destination);
                commandList.clearRenderTargetView(destination->getRtv(), blackColor);
                commandList.setGraphicsRoot32BitConstant(0, postProcessData.cb);
                commandList.setSrvInDescriptorTable(1, 0, *source);
                commandList.draw(6u);

                renderTargets.swapResources();

                // Render vertical pass
                const bool lastPass = lastPostProcess && (passIndex == postProcessData.passCount - 1);
                getAndPrepareSourceAndDestinationForPostProcess(commandList, renderTargets, false, lastPass, input, output, source, destination);
                postProcessData.cb.horizontal = false;
                commandList.OMSetRenderTargetNoDepth(*destination);
                commandList.clearRenderTargetView(destination->getRtv(), blackColor);
                commandList.setGraphicsRoot32BitConstant(0, postProcessData.cb);
                commandList.setSrvInDescriptorTable(1, 0, *source);
                commandList.draw(6u);

                if (!lastPostProcess) {
                    renderTargets.swapResources();
                }
            }
        } else {
            UNREACHABLE_CODE();
        }

        renderTargets.swapResources();
        postProcessIndex++;
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
    CommandList commandList{application.getDescriptorController(), commandQueue.getCommandAllocatorController(), nullptr};
    auto &backBuffer = swapChain.getCurrentBackBuffer();
    commandQueue.performResourcesDeletion();
    inspectObjectsNotReady();

    // Constant settings
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    // Render shadow maps
    renderShadowMaps(swapChain, renderData, commandList);

    // Render 3D
    const auto postProcessesCount = getEnabledPostProcessesCount();
    auto &renderDeferredOutput = postProcessesCount == 0 ? backBuffer : renderData.getPostProcessRenderTargets().getDestination();
    renderDeferred(swapChain, renderData, commandList, renderDeferredOutput, *postProcessVB);
    renderData.getPostProcessRenderTargets().swapResources();

    // Render post processes
    if (postProcessesCount != 0) {
        renderPostProcesses(postProcesses, commandList, *postProcessVB,
                            renderData.getPostProcessRenderTargets().getSource(), renderData.getPostProcessRenderTargets(), backBuffer,
                            postProcessesCount, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    }

    // Bloom blur
    renderPostProcesses(renderData.getPostProcessesForBloom(), commandList, *postProcessVB,
                        renderData.getBloomMap(), renderData.getPostProcessRenderTargets(), renderData.getBloomMap(),
                        1, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));

    // Apply bloom
    PostProcessApplyBloomCB cb = {};
    cb.screenWidth = static_cast<float>(swapChain.getWidth());
    cb.screenHeight = static_cast<float>(swapChain.getHeight());
    commandList.transitionBarrier(renderData.getBloomMap(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_APPLY_BLOOM);
    commandList.setGraphicsRoot32BitConstant(0, cb);
    commandList.setSrvInDescriptorTable(1, 0, renderData.getBloomMap());
    commandList.OMSetRenderTargetNoDepth(backBuffer);
    commandList.IASetVertexBuffer(*postProcessVB);
    commandList.draw(6u);

    // If there are no D2D content to render, there will be no implicit transition to present, hence the manual barrier
    if (texts.empty()) {
        commandList.transitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
    }

    // Close command list and submit it to the GPU
    commandList.close();
    std::vector<CommandList *> commandLists{&commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);

    // D2D
    if (!texts.empty()) {
        renderD2DTexts(swapChain);
    }

    // Present (swap back buffers) and wait for next frame's fence
    swapChain.present(fenceValue);
    commandQueue.getFence().waitOnCpu(swapChain.getFenceValueForCurrentBackBuffer());
}
