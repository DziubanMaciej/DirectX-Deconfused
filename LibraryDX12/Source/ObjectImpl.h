#pragma once

#include "DXD/Mesh.h"
#include "DXD/Object.h"

class ObjectImpl : public DXD::Object {
protected:
    friend class DXD::Object;
    ObjectImpl();
    ~ObjectImpl();

public:
    void setMesh(DXD::Mesh *mesh) { this->mesh = mesh; }
    void setPosition(FLOAT x, FLOAT y, FLOAT z);
    void setPosition(XMFLOAT3 pos) { this->position = pos; }
    XMFLOAT3 getPosition() { return position; }
    DXD::Mesh *getMesh() { return mesh; }

protected:
    DXD::Mesh *mesh;
    XMFLOAT3 position;
};
