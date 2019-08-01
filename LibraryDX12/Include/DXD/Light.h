#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/DirectXMath.h"
#include "DXD/ExternalHeadersWrappers/windows.h"
#include <memory>

namespace DXD {

class EXPORT Light : NonCopyableAndMovable {
public:
    virtual void setPosition(FLOAT x, FLOAT y, FLOAT z) = 0;
    virtual void setPosition(XMFLOAT3 pos) = 0;
    virtual XMFLOAT3 getPosition() const = 0;

    virtual void setColor(FLOAT r, FLOAT g, FLOAT b) = 0;
    virtual void setColor(XMFLOAT3 rgb) = 0;
    virtual XMFLOAT3 getColor() const = 0;

	virtual void setDirection(FLOAT x, FLOAT y, FLOAT z) = 0;
    virtual void setDirection(XMFLOAT3 xyz) = 0;
    virtual XMFLOAT3 getDirection() const = 0;

    virtual void setPower(float power) = 0;
    virtual float getPower() const = 0;

    virtual ~Light() = default;
	static std::unique_ptr<Light> create();

protected:
    Light() = default;
};

} // namespace DXD
