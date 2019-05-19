#pragma once

#include "DXD/Export.h"
#include "DXD/ExternalHeadersWrappers/DirectXMath.h"
#include <memory>
#include <string>

namespace DXD {
class EXPORT Camera {
public:
    virtual void setEyePosition(float x, float y, float z) = 0;
    virtual void setEyePosition(XMVECTOR vec) = 0;
    virtual XMVECTOR getEyePosition() = 0;

    virtual void setFocusPoint(float x, float y, float z) = 0;
    virtual void setFocusPoint(XMVECTOR vec) = 0;
    virtual XMVECTOR getFocusPoint() = 0;

    virtual void setUpDirection(float x, float y, float z) = 0;
    virtual void setUpDirection(XMVECTOR vec) = 0;
    virtual XMVECTOR getUpDirection() = 0;

    virtual void setFovAngleY(float val) = 0;
    virtual void setFovAngleYDeg(float val) = 0;
    virtual float getFovAngleY() = 0;
    virtual float getFovAngleYDeg() = 0;

    virtual void setNearZ(float val) = 0;
    virtual float getNearZ() = 0;

    virtual void setFarZ(float val) = 0;
    virtual float getFarZ() = 0;

    virtual ~Camera() = default;
    static std::unique_ptr<Camera> create();

protected:
    Camera() = default;
};
} // namespace DXD
