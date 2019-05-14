#pragma once

#include "DXD/Mesh.h"
#include "DXD/Object.h"
#include "MeshImpl.h"

class ObjectImpl : public DXD::Object {
protected:
    friend class DXD::Object;
    ObjectImpl();
    ~ObjectImpl();

public:
    void setMesh(DXD::Mesh &mesh) { this->mesh = static_cast<MeshImpl *>(&mesh); }
    void setPosition(FLOAT x, FLOAT y, FLOAT z);
    void setPosition(XMFLOAT3 pos) { this->position = pos; }
    XMFLOAT3 getPosition() { return position; }
    MeshImpl *getMesh() { return mesh; }

protected:
    MeshImpl *mesh;
    XMFLOAT3 position;
};
