#pragma once

#include "Api/CameraImpl.h"
#include "Api/ObjectImpl.h"

#include "DXD/Scene.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <set>

class ApplicationImpl;
class SwapChain;
class WindowImpl;
class ObjectImpl;
class CameraImpl;

class SceneImpl : public DXD::Scene {
protected:
    friend class DXD::Scene;
    SceneImpl();

public:
    void setBackgroundColor(float r, float g, float b) override;
    void addObject(DXD::Object &object) override;
    bool removeObject(DXD::Object &object) override;
    void setCamera(DXD::Camera &camera) override;
    virtual DXD::Camera *getCamera() override;

    void render(ApplicationImpl &application, SwapChain &swapChain);

protected:
    FLOAT backgroundColor[3];
    std::set<ObjectImpl *> objects; // TODO might not be the best data structure for that
    CameraImpl *camera;
};
