#include "SceneImpl.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "CommandList/CommandQueue.h"
#include "Resource/ConstantBuffers.h"
#include "Utility/ThrowIfFailed.h"
#include "Window/WindowImpl.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <algorithm>
#include <cassert>

namespace DXD {
std::unique_ptr<Scene> Scene::create(DXD::Application &application) {
    return std::unique_ptr<Scene>{new SceneImpl(application)};
}
} // namespace DXD

SceneImpl::SceneImpl(DXD::Application &application)
    : application(*static_cast<ApplicationImpl *>(&application)),
      lightConstantBuffer(this->application.getDevice(), this->application.getDescriptorController(), sizeof(SimpleConstantBuffer)) {

    ID3D12DevicePtr device = this->application.getDevice();
    auto &commandQueue = this->application.getDirectCommandQueue();

    // Record command list for GPU upload
    CommandList commandList{commandQueue.getCommandAllocatorManager(), nullptr};
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
    objects.insert(static_cast<ObjectImpl *>(&object));
}

bool SceneImpl::removeObject(DXD::Object &object) {
    const auto elementsRemoved = objects.erase(static_cast<ObjectImpl *>(&object));
    assert(elementsRemoved < 2u);
    return elementsRemoved == 1;
}

void SceneImpl::setCamera(DXD::Camera &camera) {
    this->camera = static_cast<CameraImpl *>(&camera);
}

DXD::Camera *SceneImpl::getCamera() {
    return camera;
}

void SceneImpl::renderShadowMaps(ApplicationImpl &application, SwapChain &swapChain, CommandList &commandList) {
    for (int i = 0; i < 8; i++) {
        swapChain.getShadowMap(i).transitionBarrierSingle(commandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }

    CD3DX12_VIEWPORT viewportSM(0.0f, 0.0f, 2048.0f, 2048.0f);
    commandList.RSSetViewport(viewportSM);

    commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_SM_NORMAL);

    int lightIdx = 0;

    for (LightImpl *light : lights) {
        commandList.OMSetRenderTargetDepthOnly(swapChain.getShadowMapDescriptor().getCpuHandle(lightIdx), swapChain.getShadowMap(lightIdx).getResource());
        commandList.clearDepthStencilView(swapChain.getShadowMapDescriptor().getCpuHandle(lightIdx), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

        // View projection matrix
        camera->setAspectRatio(1.0f);
        const XMMATRIX smViewMatrix = XMMatrixLookAtLH(XMVectorSet(light->getPosition().x, light->getPosition().y, light->getPosition().z, 1.f), XMVectorSet(light->getDirection().x, light->getDirection().y, light->getDirection().z, 1.f), XMVectorSet(0, 1, 0, 0.f));
        const XMMATRIX smProjectionMatrix = XMMatrixPerspectiveFovLH(90, 1, 0.1f, 160.0f);
        light->smViewProjectionMatrix = XMMatrixMultiply(smViewMatrix, smProjectionMatrix);

        for (ObjectImpl *object : objects) {
            MeshImpl &mesh = *object->getMesh();
            if (!mesh.isUploadInProgress() && mesh.getMeshType() == (MeshImpl::NORMALS | MeshImpl::TRIANGLE_STRIP)) {
                commandList.IASetVertexBuffer(*mesh.getVertexBuffer());
                commandList.IASetPrimitiveTopologyTriangleList();

                SMmvp smmvp;
                smmvp.modelViewProjectionMatrix = XMMatrixMultiply(object->getModelMatrix(), light->smViewProjectionMatrix);

                commandList.setGraphicsRoot32BitConstant(0, smmvp);

                commandList.draw(static_cast<UINT>(mesh.getVerticesCount()));
            }
        }
        lightIdx++;
    }

    for (int i = 0; i < 8; i++) {
        swapChain.getShadowMap(i).transitionBarrierSingle(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
}

void SceneImpl::renderForward(ApplicationImpl &application, SwapChain &swapChain, CommandList &commandList) {
    CD3DX12_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(swapChain.getWidth()), static_cast<float>(swapChain.getHeight()));
    commandList.RSSetViewport(viewport);

    // Transition to RENDER_TARGET
    swapChain.getPostProcessRenderTarget().transitionBarrierSingle(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.OMSetRenderTarget(swapChain.getPostProcessRtvDescriptor().getCpuHandle(), swapChain.getPostProcessRenderTarget().getResource(),
                                  swapChain.getDepthStencilBufferDescriptor(), swapChain.getDepthStencilBuffer()->getResource());

    // Render (clear color)
    commandList.clearRenderTargetView(swapChain.getPostProcessRtvDescriptor().getCpuHandle(), backgroundColor);
    commandList.clearDepthStencilView(swapChain.getDepthStencilBufferDescriptor(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

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
        MeshImpl &mesh = *object->getMesh();

        if (!mesh.isUploadInProgress() && mesh.getMeshType() == (MeshImpl::TRIANGLE_STRIP | MeshImpl::TRIANGLE_STRIP)) {
            commandList.IASetVertexBuffer(*mesh.getVertexBuffer());
            commandList.IASetIndexBuffer(*mesh.getIndexBuffer());
            commandList.IASetPrimitiveTopologyTriangleList();

            ModelMvp mmvp;
            mmvp.modelMatrix = object->getModelMatrix();
            mmvp.modelViewProjectionMatrix = XMMatrixMultiply(mmvp.modelMatrix, vpMatrix);

            commandList.setGraphicsRoot32BitConstant(0, mmvp);

            ObjectProperties op;
            op.objectColor = object->getColor();
            op.objectSpecularity = object->getSpecularity(); //Not supported in objects without normals

            commandList.setGraphicsRoot32BitConstant(1, op);

            commandList.drawIndexed(static_cast<UINT>(mesh.getIndicesCount()));
        }
    }

    //Draw NORMAL
    commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_NORMAL);
    commandList.setCbvSrvUavDescriptorTable(2, 0, lightConstantBuffer.getCbvHandle(), 1);
    commandList.setCbvSrvUavDescriptorTable(2, 1, swapChain.getShadowMapSrvDescriptor(), 8);
    for (ObjectImpl *object : objects) {
        MeshImpl &mesh = *object->getMesh();
        if (!mesh.isUploadInProgress() && mesh.getMeshType() == (MeshImpl::NORMALS | MeshImpl::TRIANGLE_STRIP)) {
            commandList.IASetVertexBuffer(*mesh.getVertexBuffer());
            commandList.IASetPrimitiveTopologyTriangleList();

            ModelMvp mmvp;
            mmvp.modelMatrix = object->getModelMatrix();
            mmvp.modelViewProjectionMatrix = XMMatrixMultiply(mmvp.modelMatrix, vpMatrix);

            commandList.setGraphicsRoot32BitConstant(0, mmvp);

            ObjectProperties op;
            op.objectColor = object->getColor();
            op.objectSpecularity = object->getSpecularity();

            commandList.setGraphicsRoot32BitConstant(1, op);

            commandList.draw(static_cast<UINT>(mesh.getVerticesCount()));
        }
    }

    //Draw TEXTURE_NORMAL
    commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_TEXTURE_NORMAL);
    commandList.setCbvSrvUavDescriptorTable(2, 0, lightConstantBuffer.getCbvHandle(), 1);
    commandList.setCbvSrvUavDescriptorTable(2, 1, swapChain.getShadowMapSrvDescriptor(), 8);
    for (ObjectImpl *object : objects) {
        MeshImpl &mesh = *object->getMesh();
        if (!mesh.isUploadInProgress() && mesh.getMeshType() == (MeshImpl::NORMALS | MeshImpl::TRIANGLE_STRIP | MeshImpl::TEXTURE_COORDS)) {
            assert(object->getTexture() != nullptr);

            commandList.IASetVertexBuffer(*mesh.getVertexBuffer());
            commandList.IASetPrimitiveTopologyTriangleList();

            ModelMvp mmvp;
            mmvp.modelMatrix = object->getModelMatrix();
            mmvp.modelViewProjectionMatrix = XMMatrixMultiply(mmvp.modelMatrix, vpMatrix);

            commandList.setGraphicsRoot32BitConstant(0, mmvp);

            ObjectProperties op;
            op.objectColor = object->getColor();
            op.objectSpecularity = object->getSpecularity();

            commandList.setGraphicsRoot32BitConstant(1, op);

            commandList.setCbvSrvUavDescriptorTable(2, 9, object->getTextureImpl()->getSrvDescriptor(), 1);
            commandList.draw(static_cast<UINT>(mesh.getVerticesCount()));
        }
    }
}

void SceneImpl::renderPostProcess(ApplicationImpl &application, SwapChain &swapChain, CommandList &commandList, Resource &input, Resource &output) {
    // Set resource to correct state
    input.transitionBarrierSingle(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    output.transitionBarrierSingle(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // Set all the states
    commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS);
    commandList.getCommandList()->OMSetRenderTargets(1, &swapChain.getCurrentBackBufferDescriptor(), FALSE, nullptr);
    commandList.setCbvSrvUavDescriptorTable(1, 0, swapChain.getSrvDescriptor());
    commandList.clearRenderTargetView(swapChain.getCurrentBackBufferDescriptor(), backgroundColor);

    // Draw black bars
    commandList.IASetVertexBuffer(*postProcessVB);
    commandList.IASetPrimitiveTopologyTriangleList();
    PostProcessCB ppcb = {};
    ppcb.screenWidth = static_cast<float>(swapChain.getWidth());
    ppcb.screenHeight = static_cast<float>(swapChain.getHeight());
    commandList.setGraphicsRoot32BitConstant(0, ppcb);
    commandList.draw(6u);
}

void SceneImpl::render(ApplicationImpl &application, SwapChain &swapChain) {
    auto &commandQueue = application.getDirectCommandQueue();
    CommandList commandList{commandQueue.getCommandAllocatorManager(), nullptr};
    const auto &backBuffer = swapChain.getCurrentBackBuffer();
    commandQueue.performResourcesDeletion();

    // Do not scissor
    CD3DX12_RECT scissorRect(0, 0, LONG_MAX, LONG_MAX);
    commandList.RSSetScissorRect(scissorRect);

    // Render
    renderShadowMaps(application, swapChain, commandList);
    renderForward(application, swapChain, commandList);
    renderPostProcess(application, swapChain, commandList, swapChain.getPostProcessRenderTarget(), *backBuffer);

    // Transition to PRESENT
    backBuffer->transitionBarrierSingle(commandList, D3D12_RESOURCE_STATE_PRESENT);

    // Close command list
    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    std::vector<CommandList *> commandLists{&commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);

    // Present (swap back buffers) and wait for next frame's fence
    swapChain.present(fenceValue);
    commandQueue.getFence().waitOnCpu(swapChain.getFenceValueForCurrentBackBuffer());
}
