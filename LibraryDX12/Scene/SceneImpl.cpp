#include "SceneImpl.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "CommandList/CommandQueue.h"
#include "Utility/ThrowIfFailed.h"
#include "Window/WindowImpl.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <algorithm>
#include <cassert>

namespace DXD {
std::unique_ptr<Scene> Scene::create() {
    return std::unique_ptr<Scene>{new SceneImpl()};
}
} // namespace DXD

SceneImpl::SceneImpl() {
    backgroundColor[0] = 0;
    backgroundColor[1] = 0;
    backgroundColor[2] = 0;
    ambientLight[0] = 0;
    ambientLight[1] = 0;
    ambientLight[2] = 0;
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
    lights.insert(static_cast<LightImpl *>(&light));
}

bool SceneImpl::removeLight(DXD::Light &light) {
    const auto elementsRemoved = lights.erase(static_cast<LightImpl *>(&light));
    assert(elementsRemoved < 2u);
    return elementsRemoved == 1;
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

void SceneImpl::render(ApplicationImpl &application, SwapChain &swapChain) {
    auto &commandQueue = application.getDirectCommandQueue();
    CommandList commandList{commandQueue.getCommandAllocatorManager(), nullptr};
    const auto &backBuffer = swapChain.getCurrentBackBuffer();
    commandQueue.performResourcesDeletion();

    // Transition to RENDER_TARGET
    commandList.transitionBarrierSingle(backBuffer->getResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_DEFAULT);

    CD3DX12_VIEWPORT viewport(0.0f, 0.0f, (float)swapChain.getWidth(), (float)swapChain.getHeight());
    CD3DX12_RECT scissorRect(0, 0, LONG_MAX, LONG_MAX);
    commandList.RSSetScissorRect(scissorRect);
    commandList.RSSetViewport(viewport);

    commandList.OMSetRenderTarget(swapChain.getCurrentBackBufferDescriptor(), swapChain.getCurrentBackBuffer()->getResource(),
                                  swapChain.getDepthStencilBufferDescriptor(), swapChain.getDepthStencilBuffer()->getResource());

    // Render (clear color)
    commandList.clearRenderTargetView(swapChain.getCurrentBackBufferDescriptor(), backgroundColor);
    commandList.clearDepthStencilView(swapChain.getDepthStencilBufferDescriptor(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

    // View projection matrix
    float aspectRatio = (float)swapChain.getWidth() / swapChain.getHeight();
    camera->setAspectRatio(aspectRatio);
    const XMMATRIX viewMatrix = camera->getViewMatrix();
    const XMMATRIX projectionMatrix = camera->getProjectionMatrix();
    const XMMATRIX vpMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

    //SimpleConstantBuffer
    SimpleConstantBuffer *smplCbv = swapChain.getSimpleConstantBufferData();
    smplCbv->cameraPosition = XMFLOAT4(camera->getEyePosition().x, camera->getEyePosition().y, camera->getEyePosition().z, 1);
    smplCbv->lightsSize = 0;
    smplCbv->ambientLight = XMFLOAT3(ambientLight[0], ambientLight[1], ambientLight[2]);
    for (LightImpl *light : lights) {
        smplCbv->lightColor[smplCbv->lightsSize] = XMFLOAT4(light->getColor().x, light->getColor().y, light->getColor().z, light->getPower());
        smplCbv->lightPosition[smplCbv->lightsSize] = XMFLOAT4(light->getPosition().x, light->getPosition().y, light->getPosition().z, 0);
        smplCbv->lightDirection[smplCbv->lightsSize] = XMFLOAT4(light->getDirection().x, light->getDirection().y, light->getDirection().z, 0);
        smplCbv->lightsSize++;
        if (smplCbv->lightsSize >= 8) {
            break;
        }
    }

    memcpy(swapChain.getSimpleCbvDataBegin(), swapChain.getSimpleConstantBufferData(), sizeof(*swapChain.getSimpleConstantBufferData()));

    ID3D12DescriptorHeap *ppHeaps[] = {swapChain.getCbvDescriptorHeap().Get()};
    commandList.getCommandList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    commandList.getCommandList()->SetGraphicsRootDescriptorTable(2, swapChain.getCbvDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

    // Draw DEFAULT
    for (ObjectImpl *object : objects) {
        MeshImpl &mesh = *object->getMesh();

        if (mesh.getMeshType() == MeshImpl::MeshType::TRIANGLE_STRIP) {
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

    commandList.setPipelineStateAndGraphicsRootSignature(application.getPipelineStateController(), PipelineStateController::Identifier::PIPELINE_STATE_NORMAL);

    commandList.getCommandList()->SetGraphicsRootDescriptorTable(2, swapChain.getCbvDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

    //Draw NORMAL
    for (ObjectImpl *object : objects) {
        MeshImpl &mesh = *object->getMesh();
        if (mesh.getMeshType() == MeshImpl::MeshType::TRIANGLE_STRIP_WITH_NORMALS) {
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

            commandList.drawInstanced(static_cast<UINT>(mesh.getVerticesCount() / 6), 1, 0, 0); // 6 - size of position+normal
        }
    }

    // Transition to PRESENT
    commandList.transitionBarrierSingle(backBuffer->getResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    // Close command list
    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    std::vector<CommandList *> commandLists{&commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);

    // Present (swap back buffers) and wait for next frame's fence
    swapChain.present(fenceValue);
    commandQueue.getFence().waitOnCpu(swapChain.getFenceValueForCurrentBackBuffer());
}
