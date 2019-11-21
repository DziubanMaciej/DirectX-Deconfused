#pragma once

#include "DXD/Mesh.h"
#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/DirectXMath.h>
#include <DXD/ExternalHeadersWrappers/windows.h>
#include <memory>
#include <string>

namespace DXD {

class Texture;

/// \brief Physical object in 3D space
///
/// Contains information about location of the object in space, as well
/// as its related objects, like the geometry, textures and other visual
/// properties
class EXPORT Object : NonCopyableAndMovable {
public:
    /// @{
    virtual void setPosition(FLOAT x, FLOAT y, FLOAT z) = 0;
    virtual void setPosition(XMFLOAT3 pos) = 0;
    virtual XMFLOAT3 getPosition() const = 0;
    /// @}

    /// @{
    virtual void setRotation(XMFLOAT3 axis, float angle) = 0;
    virtual void setRotation(float roll, float yaw, float pitch) = 0;
    virtual XMFLOAT3 getRotationQuaternion() const = 0;
    /// @}

    /// @{
    virtual void setRotationOrigin(FLOAT x, FLOAT y, FLOAT z) = 0;
    virtual void setRotationOrigin(XMFLOAT3 pos) = 0;
    virtual XMFLOAT3 getRotationOrigin() const = 0;
    /// @}

    /// @{
    virtual void setScale(FLOAT x, FLOAT y, FLOAT z) = 0;
    virtual void setScale(XMFLOAT3 scale) = 0;
    virtual XMFLOAT3 getScale() const = 0;
    /// @}

    /// @{
    virtual void setColor(FLOAT r, FLOAT g, FLOAT b) = 0;
    virtual XMFLOAT3 getColor() const = 0;
    /// @}

    /// @{
    virtual void setSpecularity(float s) = 0;
    virtual float getSpecularity() const = 0;
    /// @}

    /// @{
    virtual void setBloomFactor(float bloomFactor) = 0;
    virtual float getBloomFactor() const = 0;
    /// @}

    /// @{
    virtual void setTexture(Texture *texture) = 0;
    virtual const Texture *getTexture() = 0;
    /// @}

    /// @{
    virtual void setNormalMap(Texture *texture) = 0;
    virtual const Texture *getNormalMap() = 0;
    /// @}

    /// @{
    virtual void setTextureScale(float u, float v) = 0;
    virtual void setTextureScale(XMFLOAT2 uv) = 0;
    virtual XMFLOAT2 getTextureScale() const = 0;
    /// @}

    /// Factory method used to create Object instances
    /// \param geometry associated with the object
    /// \return Object instance
    static std::unique_ptr<Object> create(DXD::Mesh &mesh);
    virtual ~Object() = default;

protected:
    Object() = default;
};

} // namespace DXD
