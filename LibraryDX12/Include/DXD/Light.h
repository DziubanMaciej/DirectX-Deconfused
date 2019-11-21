#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/DirectXMath.h>
#include <DXD/ExternalHeadersWrappers/windows.h>
#include <memory>

namespace DXD {

/// \brief %Light source description
///
/// Data of light source to place in 3D scene. Light can be in focus mode or direction
/// mode just like the Camera class.
class EXPORT Light : NonCopyableAndMovable {
public:
    ///@{
    virtual void setPosition(float x, float y, float z) = 0;
    virtual void setPosition(XMFLOAT3 vec) = 0;
    virtual XMFLOAT3 getPosition() const = 0;
    ///@}

    ///@{

    /// Sets looking direction and puts Light in direction mode
    virtual void setDirection(float x, float y, float z) = 0;
    /// Sets looking direction and puts Light in direction mode
    virtual void setDirection(XMFLOAT3 vec) = 0;
    /// Retrieves look direction. If the Light is in focus mode, direction is calculated
    virtual XMFLOAT3 getDirection() const = 0;
    ///@}

    ///@{

    /// Sets focus point and puts Light in focus mode
    virtual void setFocusPoint(float x, float y, float z) = 0;
    /// Sets focus point and puts Light in focus mode
    virtual void setFocusPoint(XMFLOAT3 vec) = 0;
    /// Retrieves focus point. If the Light is in direction mode, focus point is calculated
    virtual XMFLOAT3 getFocusPoint() const = 0;
    ///@}

    ///@{
    virtual void setColor(FLOAT r, FLOAT g, FLOAT b) = 0;
    virtual void setColor(XMFLOAT3 rgb) = 0;
    virtual XMFLOAT3 getColor() const = 0;
    ///@}

    ///@{
    virtual void setPower(float power) = 0;
    virtual float getPower() const = 0;
    ///@}

    enum class LightType {
        /// Conical shape of light emitted
        SPOT_LIGHT,
        /// Columnar shape of light emitted
        DIRECTIONAL_LIGHT,
    };

    /// @{
    virtual void setType(LightType type) = 0;
    virtual LightType getType() = 0;
    /// @}

    /// Factory method used to create Light instances
    /// \param type type of light shape to create. Can be changed later
    /// \return Light instance
    static std::unique_ptr<Light> create(LightType type);
    virtual ~Light() = default;

protected:
    Light() = default;
};

} // namespace DXD
