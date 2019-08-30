#include "SceneImpl.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "CommandList/CommandQueue.h"
#include "Resource/ConstantBuffers.h"
#include "Scene/RenderData.h"
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
      lightConstantBuffer(application.getDevice(), application.getDescriptorController(), sizeof(SimpleConstantBuffer)) {

    ID3D12DevicePtr device = application.getDevice();
    auto &commandQueue = application.getDirectCommandQueue();

    // Record command list for GPU upload
    CommandList commandList{application.getDescriptorController(), commandQueue.getCommandAllocatorController(), nullptr};
    postProcessVB = std::make_unique<VertexBuffer>(device, commandList, postProcessSquare, 6, 12);

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

void SceneImpl::getAndPrepareSourceAndDestinationForPostProcess(CommandList &commandList, PostProcessRenderTargets &renderTargets, bool last,
                                                                Resource &finalOutput, Resource *&outSource, Resource *&outDestination) {
    // Get resources
    outSource = &renderTargets.getSource();
    outDestination = last ? &finalOutput : &renderTargets.getDestination();

    // Set states
    commandList.transitionBarrier(*outSource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(*outDestination, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

// --------------------------------------------------------------------------- Render methods

void SceneImpl::renderShadowMaps(SwapChain &swapChain, RenderData &renderData, CommandList &commandList) {
    const auto shadowMapsUsed = std::min(size_t{8u}, lights.size());

    for (int i = 0; i < shadowMapsUsed; i++) {
        commandList.transitionBarrier(renderData.getShadowMap(i), D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }

    commandList.RSSetViewport(0.f, 0.f, 2048.f, 2048.f);

    int lightIdx = 0;

    for (LightImpl *light : lights) {
        commandList.OMSetRenderTargetDepthOnly(renderData.getShadowMap(lightIdx).getDsv(), renderData.getShadowMap(lightIdx).getResource());
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
        commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_SM_NORMAL);
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
        commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_SM_TEXTURE_NORMAL);
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

void SceneImpl::renderForward(SwapChain &swapChain, RenderData &renderData, CommandList &commandList, Resource &output) {
    commandList.RSSetViewport(0.f, 0.f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));

    // Transition to RENDER_TARGET
    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.OMSetRenderTarget(output.getRtv(), output.getResource(),
                                  renderData.getDepthStencilBuffer().getDsv(), renderData.getDepthStencilBuffer().getResource());

    // Render (clear color)
    commandList.clearRenderTargetView(output.getRtv(), backgroundColor);
    commandList.clearDepthStencilView(renderData.getDepthStencilBuffer().getDsv(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

    // View projection matrix
    float aspectRatio = (float)swapChain.getWidth() / swapChain.getHeight();
    camera->setAspectRatio(aspectRatio);
    const XMMATRIX viewMatrix = camera->getViewMatrix();
    const XMMATRIX projectionMatrix = camera->getProjectionMatrix();
    const XMMATRIX vpMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

    //SimpleConstantBuffer
    auto smplCbv = lightConstantBuffer.getData<SimpleConstantBuffer>();
    smplCbv->cameraPosition = XMFLOAT4(camera->getEyePosition().x, camera->getEyePosition().y, camera->getEyePosition().z, 1);
    smplCbv->lightsSize = 0;
    smplCbv->ambientLight = XMFLOAT3(ambientLight[0], ambientLight[1], ambientLight[2]);
    for (LightImpl *light : lights) {
        smplCbv->lightColor[smplCbv->lightsSize] = XMFLOAT4(light->getColor().x, light->getColor().y, light->getColor().z, light->getPower());
        smplCbv->lightPosition[smplCbv->lightsSize] = XMFLOAT4(light->getPosition().x, light->getPosition().y, light->getPosition().z, 0);
        smplCbv->lightDirection[smplCbv->lightsSize] = XMFLOAT4(light->getDirection().x, light->getDirection().y, light->getDirection().z, 0);
        smplCbv->smViewProjectionMatrix[smplCbv->lightsSize] = light->smViewProjectionMatrix;
        smplCbv->lightsSize++;
        if (smplCbv->lightsSize >= 8) {
            break;
        }
    }
    lightConstantBuffer.upload();

    // Draw DEFAULT
    commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_DEFAULT);
    commandList.setCbvSrvUavDescriptorTable(2, 0, lightConstantBuffer.getCbvHandle(), 1);
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
    }

    //Draw NORMAL
    commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_NORMAL);
    commandList.setCbvSrvUavDescriptorTable(2, 0, lightConstantBuffer.getCbvHandle(), 1);
    for (auto shadowMapIndex = 0u; shadowMapIndex < 8; shadowMapIndex++) {
        commandList.setCbvSrvUavDescriptorTable(2, shadowMapIndex + 1, renderData.getShadowMap(shadowMapIndex).getSrv(), 1);
    }
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
    commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_TEXTURE_NORMAL);
    commandList.setCbvSrvUavDescriptorTable(2, 0, lightConstantBuffer.getCbvHandle(), 1);
    for (auto shadowMapIndex = 0u; shadowMapIndex < 8; shadowMapIndex++) {
        commandList.setCbvSrvUavDescriptorTable(2, shadowMapIndex + 2, renderData.getShadowMap(shadowMapIndex).getSrv(), 1);
    }
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
            commandList.setCbvSrvUavDescriptorTable(2, 1, texture->getSrvDescriptor(), 1);
            commandList.draw(static_cast<UINT>(mesh.getVerticesCount()));
        }
    }
}

void SceneImpl::renderPostProcesses(SwapChain &swapChain, PostProcessRenderTargets &renderTargets, CommandList &commandList,
                                    size_t enablePostProcessesCount, Resource &output) {
    // TODO skip disabled post processes
    size_t postProcessIndex = 0u;
    Resource *source{}, *destination{};
    for (PostProcessImpl *postProcess : postProcesses) {
        if (!postProcess->isEnabled()) {
            continue;
        }

        // Get resources
        const bool lastPostProcess = (postProcessIndex == enablePostProcessesCount - 1);
        getAndPrepareSourceAndDestinationForPostProcess(commandList, renderTargets, lastPostProcess, output, source, destination);

        // Render post process base on its type
        if (postProcess->getType() == PostProcessImpl::Type::CONVOLUTION) {
            // Prepare constant buffer
            auto &postProcessData = postProcess->getData().convolution;
            postProcessData.screenWidth = static_cast<float>(swapChain.getWidth());
            postProcessData.screenHeight = static_cast<float>(swapChain.getHeight());

            // Set state
            commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_CONVOLUTION);
            commandList.setGraphicsRoot32BitConstant(0, postProcessData);
            commandList.setCbvSrvUavDescriptorTable(1, 0, source->getSrv(), 1);
            commandList.OMSetRenderTargetNoDepth(destination->getRtv(), destination->getResource());
            commandList.IASetVertexBuffer(*postProcessVB);

            // Render
            commandList.draw(6u);
        } else if (postProcess->getType() == PostProcessImpl::Type::BLACK_BARS) {
            // Prepare constant buffer
            auto &postProcessData = postProcess->getData().blackBars;
            postProcessData.screenWidth = static_cast<float>(swapChain.getWidth());
            postProcessData.screenHeight = static_cast<float>(swapChain.getHeight());

            // Set state
            commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_BLACK_BARS);
            commandList.setGraphicsRoot32BitConstant(0, postProcessData);
            commandList.setCbvSrvUavDescriptorTable(1, 0, source->getSrv(), 1);
            commandList.OMSetRenderTargetNoDepth(destination->getRtv(), destination->getResource());
            commandList.IASetVertexBuffer(*postProcessVB);

            source->getResource()->SetName(L"SOURCE");
            destination->getResource()->SetName(L"DSTNATION");

            // Render
            commandList.draw(6u);
        } else if (postProcess->getType() == PostProcessImpl::Type::LINEAR_COLOR_CORRECTION) {
            // Prepare onstant buffer
            auto &postProcessData = postProcess->getData().linearColorCorrection;
            postProcessData.screenWidth = static_cast<float>(swapChain.getWidth());
            postProcessData.screenHeight = static_cast<float>(swapChain.getHeight());

            // Set state
            commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_LINEAR_COLOR_CORRECTION);
            commandList.setGraphicsRoot32BitConstant(0, postProcessData);
            commandList.setCbvSrvUavDescriptorTable(1, 0, source->getSrv(), 1);
            commandList.OMSetRenderTargetNoDepth(destination->getRtv(), destination->getResource());
            commandList.IASetVertexBuffer(*postProcessVB);

            // Render
            commandList.draw(6u);
        } else if (postProcess->getType() == PostProcessImpl::Type::GAUSSIAN_BLUR) {
            // Prepare constant buffer
            auto &postProcessData = postProcess->getData().gaussianBlur;
            postProcessData.cb.screenWidth = static_cast<float>(swapChain.getWidth());
            postProcessData.cb.screenHeight = static_cast<float>(swapChain.getHeight());

            // Set cross-pass state
            commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR);
            commandList.IASetVertexBuffer(*postProcessVB);

            // Render all passes of the blur filter
            for (auto passIndex = 0u; passIndex < postProcessData.passCount; passIndex++) {
                // Render horizontal pass
                getAndPrepareSourceAndDestinationForPostProcess(commandList, renderTargets, false, output, source, destination);
                postProcessData.cb.horizontal = true;
                commandList.OMSetRenderTargetNoDepth(destination->getRtv(), destination->getResource());
                commandList.clearRenderTargetView(destination->getRtv(), backgroundColor);
                commandList.setGraphicsRoot32BitConstant(0, postProcessData.cb);
                commandList.setCbvSrvUavDescriptorTable(1, 0, source->getSrv(), 1);
                commandList.draw(6u);

                renderTargets.swapResources();

                // Render vertical pass
                const bool lastPass = (passIndex == postProcessData.passCount - 1);
                getAndPrepareSourceAndDestinationForPostProcess(commandList, renderTargets, lastPass, output, source, destination);
                postProcessData.cb.horizontal = false;
                commandList.OMSetRenderTargetNoDepth(destination->getRtv(), destination->getResource());
                commandList.clearRenderTargetView(destination->getRtv(), backgroundColor);
                commandList.setGraphicsRoot32BitConstant(0, postProcessData.cb);
                commandList.setCbvSrvUavDescriptorTable(1, 0, source->getSrv(), 1);
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
    auto &renderForwardOutput = postProcessesCount == 0 ? backBuffer : renderData.getPostProcessRenderTargets().getDestination();
    renderForward(swapChain, renderData, commandList, renderForwardOutput);
    renderData.getPostProcessRenderTargets().swapResources();

    // Render post processes
    if (postProcessesCount != 0) {
        renderPostProcesses(swapChain, renderData.getPostProcessRenderTargets(), commandList, postProcessesCount, backBuffer);
    }

    // Transition to PRESENT
    commandList.transitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);

    // Close command list
    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    std::vector<CommandList *> commandLists{&commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);

    // Present (swap back buffers) and wait for next frame's fence
    swapChain.present(fenceValue);
    commandQueue.getFence().waitOnCpu(swapChain.getFenceValueForCurrentBackBuffer());
}
