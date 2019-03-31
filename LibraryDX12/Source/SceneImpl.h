#pragma once

#include "DXD/Scene.h"
#include "DXD/ExternalHeadersWrappers/d3d12.h"

class ApplicationImpl;
class SwapChain;
class WindowImpl;

class SceneImpl : public DXD::Scene {
protected:
    friend class DXD::Scene;
    SceneImpl();

public:
    void setBackgroundColor(float r, float g, float b) override;

    void render(ApplicationImpl &application, SwapChain &swapChain);

protected:
    FLOAT backgroundColor[3];
};
