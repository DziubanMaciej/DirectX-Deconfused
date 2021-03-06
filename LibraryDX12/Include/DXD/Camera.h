#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/DirectXMath.h>
#include <memory>

namespace DXD {

/// \brief Encapulates view and projection data
///
/// The Camera class holds information about where the viewer is, what he looks at,
/// how are they rotated and how much can they see. Camera can operate in two modes
/// 1. Focus mode is when the viewer looks at one given point, Camera will rotate as
/// he moves.
/// 2. Direction mode is when the viewer looks in a given direction, Camera will not
/// rotate as he moves.
class EXPORT Camera : NonCopyableAndMovable {
public:
    ///@{
    virtual void setEyePosition(float x, float y, float z) = 0;
    virtual void setEyePosition(XMFLOAT3 vec) = 0;
    virtual XMFLOAT3 getEyePosition() const = 0;
    ///@}

    ///@{
    virtual void setFocusPoint(float x, float y, float z) = 0;
    virtual void setFocusPoint(XMFLOAT3 vec) = 0;
    virtual XMFLOAT3 getFocusPoint() const = 0;
    ///@}

    ///@{
    virtual void setLookDirection(float x, float y, float z) = 0;
    virtual void setLookDirection(XMFLOAT3 vec) = 0;
    virtual XMFLOAT3 getLookDirection() const = 0;
    ///@}

    ///@{
    virtual void setUpDirection(float x, float y, float z) = 0;
    virtual void setUpDirection(XMFLOAT3 vec) = 0;
    virtual XMFLOAT3 getUpDirection() const = 0;
    ///@}

    ///@{
    virtual void setFovAngleY(float val) = 0;
    virtual void setFovAngleYDeg(float val) = 0;
    virtual float getFovAngleY() const = 0;
    virtual float getFovAngleYDeg() const = 0;
    ///@}

    ///@{
    virtual void setNearZ(float val) = 0;
    virtual float getNearZ() const = 0;
    virtual void setFarZ(float val) = 0;
    virtual float getFarZ() const = 0;
    ///@}

    /// Factory function used to create Camera instance.
    /// \return Application instance
    static std::unique_ptr<Camera> create();
    virtual ~Camera() = default;

protected:
    Camera() = default;
};
} // namespace DXD
