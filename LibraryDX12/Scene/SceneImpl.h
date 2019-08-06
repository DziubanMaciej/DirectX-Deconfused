#pragma once

#include "Resource/ConstantBuffer.h"
#include "Scene/CameraImpl.h"
#include "Scene/LightImpl.h"
#include "Scene/ObjectImpl.h"

#include "DXD/Scene.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <set>

class ApplicationImpl;
class SwapChain;
class WindowImpl;
class LightImpl;
class ObjectImpl;
class CameraImpl;

class SceneImpl : public DXD::Scene {
protected:
    friend class DXD::Scene;
    SceneImpl(DXD::Application &application);

public:
    void setBackgroundColor(float r, float g, float b) override;
    void setAmbientLight(float r, float g, float b) override;
    void addLight(DXD::Light &light) override;
    bool removeLight(DXD::Light &light) override;
    void addObject(DXD::Object &object) override;
    bool removeObject(DXD::Object &object) override;
    void setCamera(DXD::Camera &camera) override;
    virtual DXD::Camera *getCamera() override;

    void render(ApplicationImpl &application, SwapChain &swapChain);

protected:
    // Context
    ApplicationImpl &application;

    // Buffers
    ConstantBuffer lightConstantBuffer;
    std::unique_ptr<VertexBuffer> postProcessVB;

    // Data set by user
    FLOAT backgroundColor[3] = {};
    FLOAT ambientLight[3] = {};
    std::set<LightImpl *> lights;
    std::set<ObjectImpl *> objects; // TODO might not be the best data structure for that
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
