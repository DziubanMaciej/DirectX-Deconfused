#pragma once

#include "DXD/Export.h"
#include "DXD/Mesh.h"
#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/DirectXMath.h"
#include "DXD/ExternalHeadersWrappers/windows.h"
#include <memory>
#include <string>

namespace DXD {

class Texture;

class EXPORT Object : NonCopyableAndMovable {
public:
    virtual void setMesh(DXD::Mesh &mesh) = 0;

    virtual void setPosition(FLOAT x, FLOAT y, FLOAT z) = 0;
    virtual void setPosition(XMFLOAT3 pos) = 0;
    virtual XMFLOAT3 getPosition() const = 0;

    virtual void setRotation(XMFLOAT3 axis, float angle) = 0;
    virtual void setRotation(float roll, float yaw, float pitch) = 0;
    virtual XMFLOAT3 getRotationQuaternion() const = 0;

    virtual void setRotationOrigin(FLOAT x, FLOAT y, FLOAT z) = 0;
    virtual void setRotationOrigin(XMFLOAT3 pos) = 0;
    virtual XMFLOAT3 getRotationOrigin() const = 0;

    virtual void setScale(FLOAT x, FLOAT y, FLOAT z) = 0;
    virtual void setScale(XMFLOAT3 scale) = 0;
    virtual XMFLOAT3 getScale() const = 0;

    virtual void setColor(FLOAT r, FLOAT g, FLOAT b) = 0;
    virtual XMFLOAT3 getColor() const = 0;

    virtual void setSpecularity(float s) = 0;
    virtual float getSpecularity() const = 0;

    virtual void setTexture(Texture *texture) = 0;
    virtual const Texture *getTexture() = 0;

    virtual ~Object() = default;
    static std::unique_ptr<Object> create();

protected:
    Object() = default;
};

} // namespace DXD
