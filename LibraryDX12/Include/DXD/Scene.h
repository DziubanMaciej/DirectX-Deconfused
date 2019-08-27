#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"
#include <memory>

namespace DXD {

class Text;
class Light;
class Object;
class Camera;
class PostProcess;

class EXPORT Scene : NonCopyableAndMovable {
public:
    virtual void setBackgroundColor(float r, float g, float b) = 0;
    virtual void setAmbientLight(float r, float g, float b) = 0;
    virtual void addLight(DXD::Light &light) = 0;
    virtual void addPostProcess(DXD::PostProcess &postProcess) = 0;
    virtual bool removeLight(DXD::Light &light) = 0;
    virtual void addObject(DXD::Object &object) = 0;
    virtual void addText(DXD::Text &text) = 0;
    virtual bool removeObject(DXD::Object &object) = 0;
    virtual void setCamera(DXD::Camera &camera) = 0;
    virtual DXD::Camera *getCamera() = 0;

    virtual ~Scene() = default;
    static std::unique_ptr<Scene> create(DXD::Application &application);

protected:
    Scene() = default;
};

} // namespace DXD
