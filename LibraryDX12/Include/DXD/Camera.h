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
    /// \name Camera position accessors
    ///@{

    /// Sets the camera position
    virtual void setEyePosition(float x, float y, float z) = 0;
    /// Sets the camera position
    virtual void setEyePosition(XMFLOAT3 vec) = 0;
    /// Retrieves the camera position
    virtual XMFLOAT3 getEyePosition() const = 0;
    ///@}

    /// \name Focus point accessors
    ///@{

    /// Sets focus the point and puts Camera in focus mode
    virtual void setFocusPoint(float x, float y, float z) = 0;
    /// Sets focus the point and puts Camera in focus mode
    virtual void setFocusPoint(XMFLOAT3 vec) = 0;
    /// Retrieves focus point. If the Camera is in direction mode, focus point is calculated.
    virtual XMFLOAT3 getFocusPoint() const = 0;
    ///@}

    /// \name Look direction accessors
    ///@{

    /// Sets looking direction and puts Camera in direction mode
    virtual void setLookDirection(float x, float y, float z) = 0;
    /// Sets looking direction and puts Camera in direction mode
    virtual void setLookDirection(XMFLOAT3 vec) = 0;
    /// Retrieves look direction. If the Camera is in focus mode, direction is calculated
    virtual XMFLOAT3 getLookDirection() const = 0;
    ///@}

    /// \name Up direction accessors
    ///@{

    /// Sets up direction
    virtual void setUpDirection(float x, float y, float z) = 0;
    /// Sets up direction
    virtual void setUpDirection(XMFLOAT3 vec) = 0;
    /// Retrieves up direction
    virtual XMFLOAT3 getUpDirection() const = 0;
    ///@}

    /// \name Field of view accessors
    ///@{

    /// Sets FoV of the Camera
    virtual void setFovAngleY(float val) = 0;
    /// Sets FoV of the Camera
    virtual void setFovAngleYDeg(float val) = 0;
    /// Retrieves FoV of the Camera
    virtual float getFovAngleY() const = 0;
    /// Retrieves FoV of the Camera
    virtual float getFovAngleYDeg() const = 0;
    ///@}

    /// \brief Z planes accessors
    ///@{

    /// Sets near Z plane
    virtual void setNearZ(float val) = 0;
    /// Retrieves near Z plane
    virtual float getNearZ() const = 0;
    /// Sets far Z plane
    virtual void setFarZ(float val) = 0;
    /// Retrieves far Z plane
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
