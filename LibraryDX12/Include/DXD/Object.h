#pragma once

#include "DXD/Export.h"
#include "DXD/ExternalHeadersWrappers/DirectXMath.h"
#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <memory>
#include <string>

namespace DXD {
class EXPORT Object {
public:
    virtual void setMesh(DXD::Mesh &mesh) = 0;
    virtual void setPosition(FLOAT x, FLOAT y, FLOAT z) = 0;
    virtual void setPosition(XMFLOAT3 pos) = 0;
    virtual XMFLOAT3 getPosition() = 0;

    virtual ~Object() = default;
    static std::unique_ptr<Object> create();

protected:
    Object() = default;
};
} // namespace DXD
