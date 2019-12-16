#include "SceneImpl.h"

#include "Renderer/Renderer.h"
#include "Resource/ConstantBuffer.h"
#include "Scene/CameraImpl.h"
#include "Scene/LightImpl.h"
#include "Scene/ObjectImpl.h"
#include "Scene/PostProcessImpl.h"
#include "Scene/SpriteImpl.h"
#include "Scene/TextImpl.h"

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

void SceneImpl::render(SwapChain &swapChain, RenderData &renderData) {
    Renderer renderer{swapChain, renderData, *this};
    renderer.render();
}

// --------------------------------------------------------------------------- Accessors

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
