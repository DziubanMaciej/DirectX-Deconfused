#pragma once

#include "DXD/Mesh.h"
#include "DXD/Object.h"
#include "MeshImpl.h"

class ObjectImpl : public DXD::Object {
protected:
    friend class DXD::Object;

public:
    void setMesh(DXD::Mesh &mesh) { this->mesh = static_cast<MeshImpl *>(&mesh); }
    MeshImpl *getMesh() { return mesh; }
    const XMMATRIX &getModelMatrix();

    void setPosition(FLOAT x, FLOAT y, FLOAT z) override;
    void setPosition(XMFLOAT3 pos) override;
    XMFLOAT3 getPosition() const override;

    void setRotation(XMFLOAT3 axis, float angle) override;
    void setRotation(float roll, float yaw, float pitch) override;
    XMFLOAT3 getRotationQuaternion() const override;

    void setRotationOrigin(FLOAT x, FLOAT y, FLOAT z) override;
    void setRotationOrigin(XMFLOAT3 pos) override;
    XMFLOAT3 getRotationOrigin() const override;

protected:
    MeshImpl *mesh;

    XMVECTOR position;
    XMVECTOR rotationQuaternion;
    XMVECTOR rotationOrigin;

    XMMATRIX modelMatrix;
    bool modelMatrixDirty = true;
};
