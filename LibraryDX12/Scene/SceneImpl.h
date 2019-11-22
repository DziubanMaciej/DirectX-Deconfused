#pragma once

#include "Resource/ConstantBuffer.h"
#include "Scene/CameraImpl.h"
#include "Scene/LightImpl.h"
#include "Scene/ObjectImpl.h"
#include "Scene/PostProcessImpl.h"
#include "Scene/SpriteImpl.h"
#include "Scene/TextImpl.h"

#include "DXD/Scene.h"

#include <DXD/ExternalHeadersWrappers/d3d12.h>
#include <set>
#include <vector>

class ApplicationImpl;
class SwapChain;
class WindowImpl;
class LightImpl;
class ObjectImpl;
class CameraImpl;
class RenderData;
struct AlternatingResources;

class SceneImpl : public DXD::Scene {
protected:
    friend class DXD::Scene;
    SceneImpl();

public:
    void setBackgroundColor(float r, float g, float b) override;
    void setAmbientLight(float r, float g, float b) override;

    void addLight(DXD::Light &light) override;
    unsigned int removeLight(DXD::Light &light) override;

    void addPostProcess(DXD::PostProcess &postProcess) override;
    unsigned int removePostProcess(DXD::PostProcess &postProcess) override;

    void addObject(DXD::Object &object) override;
    unsigned int removeObject(DXD::Object &object) override;

    void addText(DXD::Text &text) override;
    unsigned int removeText(DXD::Text &text) override;

    void addSprite(DXD::Sprite &sprite) override;
    unsigned int removeSprite(DXD::Sprite &sprite) override;

    void setCamera(DXD::Camera &camera) override;
    virtual DXD::Camera *getCamera() override;

    void render(SwapChain &swapChain, RenderData &renderData);

    Microsoft::WRL::ComPtr<ID3D12QueryHeap> queryHeap;
    std::unique_ptr<Resource> queryResult;

protected:
    // Helper for scene management
    template <typename Type, typename TypeImpl>
    uint32_t removeFromScene(std::vector<TypeImpl *> &vector, Type &object) {
        TypeImpl *objectImpl = static_cast<TypeImpl *>(&object);
        const auto sizeBefore = vector.size();
        auto removeIterator = std::remove(vector.begin(), vector.end(), objectImpl);
        vector.erase(removeIterator, vector.end());
        return static_cast<uint32_t>(sizeBefore - vector.size());
    }

    // Helpers
    void inspectObjectsNotReady();
    size_t getEnabledPostProcessesCount() const;
    static void getAndPrepareSourceAndDestinationForPostProcess(CommandList &commandList, AlternatingResources &renderTargets, bool first, bool last,
                                                                Resource *initialInput, Resource *finalOutput, Resource *&outSource, Resource *&outDestination,
                                                                bool compute = false);

    // Render methods
    void renderShadowMaps(SwapChain &swapChain, RenderData &renderData, CommandList &commandList);
    void renderGBuffer(SwapChain &swapChain, RenderData &renderData, CommandList &commandList);
    void renderSSAO(SwapChain &swapChain, RenderData &renderData, CommandList &commandList);
    void renderLighting(SwapChain &swapChain, RenderData &renderData, CommandList &commandList, Resource &output);
    void renderSSRandMerge(SwapChain &swapChain, RenderData &renderData, CommandList &commandList, Resource &output);
    static void renderPostProcess(PostProcessImpl &postProcess, CommandList &commandList, VertexBuffer &fullscreenVB,
                                  Resource *input, AlternatingResources &renderTargets, Resource *output,
                                  float screenWidth, float screenHeight);
    static void renderPostProcesses(std::vector<PostProcessImpl *> &postProcesses, CommandList &commandList, VertexBuffer &fullscreenVB,
                                    Resource *input, AlternatingResources &renderTargets, Resource *output,
                                    size_t enabledPostProcessesCount, float screenWidth, float screenHeight);
    static void renderBloom(SwapChain &swapChain, CommandList &commandList, PostProcessImpl &bloomBlurEffect, VertexBuffer &fullscreenVB,
                            Resource &bloomMap, AlternatingResources &renderTargets, Resource &output);
    void renderD2DTexts(SwapChain &swapChain);
    void renderSprite(SwapChain &swapChain, CommandList &commandList, SpriteImpl *sprite, VertexBuffer &fullscreenVB);

    // Data set by user
    FLOAT backgroundColor[3] = {};
    FLOAT ambientLight[3] = {};
    std::vector<LightImpl *> lights;
    std::set<ObjectImpl *> objects; // TODO might not be the best data structure for that
    std::set<ObjectImpl *> objectsNotReady;
    std::vector<TextImpl *> texts;
    std::vector<PostProcessImpl *> postProcesses = {};
    std::vector<SpriteImpl *> sprites = {};
    CameraImpl *camera;
};
