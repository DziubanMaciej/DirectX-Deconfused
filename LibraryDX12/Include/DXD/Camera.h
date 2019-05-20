#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"
#include "DXD/ExternalHeadersWrappers/DirectXMath.h"
#include <memory>

namespace DXD {
class EXPORT Camera : NonCopyableAndMovable {
public:
    virtual void setEyePosition(float x, float y, float z) = 0;
    virtual void setEyePosition(XMVECTOR vec) = 0;
    virtual XMVECTOR getEyePosition() const = 0;

    virtual void setFocusPoint(float x, float y, float z) = 0;
    virtual void setFocusPoint(XMVECTOR vec) = 0;
    virtual XMVECTOR getFocusPoint() const = 0;

    virtual void setUpDirection(float x, float y, float z) = 0;
    virtual void setUpDirection(XMVECTOR vec) = 0;
    virtual XMVECTOR getUpDirection() const = 0;

    virtual void setFovAngleY(float val) = 0;
    virtual void setFovAngleYDeg(float val) = 0;
    virtual float getFovAngleY() const = 0;
    virtual float getFovAngleYDeg() const = 0;

    virtual void setNearZ(float val) = 0;
    virtual float getNearZ() const = 0;

    virtual void setFarZ(float val) = 0;
    virtual float getFarZ() const = 0;

    virtual ~Camera() = default;
    static std::unique_ptr<Camera> create();

protected:
    Camera() = default;
};
} // namespace DXD
