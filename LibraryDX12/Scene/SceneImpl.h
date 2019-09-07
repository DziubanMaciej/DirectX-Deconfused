#pragma once

#include "Resource/ConstantBuffer.h"
#include "Scene/CameraImpl.h"
#include "Scene/LightImpl.h"
#include "Scene/ObjectImpl.h"
#include "Scene/PostProcessImpl.h"
#include "Scene/TextImpl.h"

#include "DXD/Scene.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <set>
#include <vector>

class ApplicationImpl;
class SwapChain;
class WindowImpl;
class LightImpl;
class ObjectImpl;
class CameraImpl;
class RenderData;
struct PostProcessRenderTargets;

class SceneImpl : public DXD::Scene {
protected:
    friend class DXD::Scene;
    SceneImpl(ApplicationImpl &application);

public:
    void setBackgroundColor(float r, float g, float b) override;
    void setAmbientLight(float r, float g, float b) override;
    void addLight(DXD::Light &light) override;
    void addText(DXD::Text &text) override;
    void addPostProcess(DXD::PostProcess &postProcess) override;
    bool removeLight(DXD::Light &light) override;
    void addObject(DXD::Object &object) override;
    bool removeObject(DXD::Object &object) override;
    void setCamera(DXD::Camera &camera) override;
    virtual DXD::Camera *getCamera() override;

    void render(SwapChain &swapChain, RenderData &renderData);

protected:
    // Helpers
    void inspectObjectsNotReady();
    size_t getEnabledPostProcessesCount() const;
    void getAndPrepareSourceAndDestinationForPostProcess(CommandList &commandList, PostProcessRenderTargets &renderTargets, bool last,
                                                         Resource &finalOutput, Resource *&outSource, Resource *&outDestination);

    // Render methods
    void renderShadowMaps(SwapChain &swapChain, RenderData &renderData, CommandList &commandList);
    void renderForward(SwapChain &swapChain, RenderData &renderData, CommandList &commandList, Resource &output);
    void renderPostProcesses(SwapChain &swapChain, PostProcessRenderTargets &renderTargets, CommandList &commandList,
                             size_t enablePostProcessesCount, Resource &output);
    void renderD2DTexts(SwapChain &swapChain);

    // Context
    ApplicationImpl &application;

    // Buffers
    ConstantBuffer lightConstantBuffer;
    std::unique_ptr<VertexBuffer> postProcessVB;

    // Data set by user
    FLOAT backgroundColor[3] = {};
    FLOAT ambientLight[3] = {};
    std::vector<LightImpl *> lights;
    std::set<ObjectImpl *> objects; // TODO might not be the best data structure for that
    std::set<ObjectImpl *> objectsNotReady;
    std::vector<TextImpl *> texts;
    std::vector<PostProcessImpl *> postProcesses = {};
    CameraImpl *camera;

    XMFLOAT3 postProcessSquare[6] =
        {
            {-1.0f, -1.0f, 0.0f},
            {-1.0f, 1.0f, 0.0f},
            {1.0f, 1.0f, 0.0f},
            {-1.0f, -1.0f, 0.0f},
            {1.0f, 1.0f, 0.0f},
            {1.0f, -1.0f, 0.0f}};
};
