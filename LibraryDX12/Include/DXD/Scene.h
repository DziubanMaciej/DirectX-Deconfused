#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"
#include <memory>

namespace DXD {
class Object;
class Camera;

class EXPORT Scene : NonCopyableAndMovable {
public:
    virtual void setBackgroundColor(float r, float g, float b) = 0;
    virtual void addObject(DXD::Object &object) = 0;
    virtual bool removeObject(DXD::Object &object) = 0;
    virtual void setCamera(DXD::Camera &camera) = 0;
    virtual DXD::Camera *getCamera() = 0;

    virtual ~Scene() = default;
    static std::unique_ptr<Scene> create();

protected:
    Scene() = default;
};
} // namespace DXD
