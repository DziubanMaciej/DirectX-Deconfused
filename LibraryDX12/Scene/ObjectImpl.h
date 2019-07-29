#pragma once

#include "Scene/MeshImpl.h"

#include "DXD/Mesh.h"
#include "DXD/Object.h"

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

    void setScale(FLOAT x, FLOAT y, FLOAT z) override;
    void setScale(XMFLOAT3 scale) override;
    XMFLOAT3 getScale() const override;

protected:
    MeshImpl *mesh;

    XMVECTOR scale = {1, 1, 1};
    XMVECTOR position = {0, 0, 0};
    XMVECTOR rotationQuaternion = XMQuaternionIdentity();
    XMVECTOR rotationOrigin = {0, 0, 0};

    XMMATRIX modelMatrix;
    bool modelMatrixDirty = true;
};
