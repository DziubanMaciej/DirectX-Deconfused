#include "ObjectImpl.h"

#include <fstream>

namespace DXD {
std::unique_ptr<Object> Object::create() {
    return std::unique_ptr<Object>{new ObjectImpl()};
}
} // namespace DXD

const XMMATRIX &ObjectImpl::getModelMatrix() {
    if (modelMatrixDirty) {
        modelMatrixDirty = false;
        modelMatrix = XMMatrixTransformation(
            XMVECTOR{0, 0, 0}, XMQuaternionIdentity(), this->scale, // scaling
            this->rotationOrigin, this->rotationQuaternion,         // rotation
            this->position                                          // translation
        );
    }
    return modelMatrix;
}

void ObjectImpl::setPosition(FLOAT x, FLOAT y, FLOAT z) {
    setPosition(XMFLOAT3{x, y, z});
}

void ObjectImpl::setPosition(XMFLOAT3 pos) {
    modelMatrixDirty = true;
    this->position = XMLoadFloat3(&pos);
}

XMFLOAT3 ObjectImpl::getPosition() const {
    return XMStoreFloat3(this->position);
}

void ObjectImpl::setRotation(XMFLOAT3 axis, float angle) {
    modelMatrixDirty = true;
    this->rotationQuaternion = XMQuaternionRotationAxis(XMLoadFloat3(&axis), angle);
}

void ObjectImpl::setRotation(float roll, float yaw, float pitch) {
    modelMatrixDirty = true;
    this->rotationQuaternion = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
}

XMFLOAT3 ObjectImpl::getRotationQuaternion() const {
    return XMStoreFloat3(this->rotationQuaternion);
}

void ObjectImpl::setRotationOrigin(FLOAT x, FLOAT y, FLOAT z) {
    setRotationOrigin(XMFLOAT3{x, y, z});
}

void ObjectImpl::setRotationOrigin(XMFLOAT3 pos) {
    modelMatrixDirty = true;
    this->rotationOrigin = XMLoadFloat3(&pos);
}

XMFLOAT3 ObjectImpl::getRotationOrigin() const {
    return XMStoreFloat3(this->rotationOrigin);
}

void ObjectImpl::setScale(FLOAT x, FLOAT y, FLOAT z) {
    setScale(XMFLOAT3{x, y, z});
}

void ObjectImpl::setScale(XMFLOAT3 scale) {
    modelMatrixDirty = true;
    this->scale = XMLoadFloat3(&scale);
}

XMFLOAT3 ObjectImpl::getScale() const {
    return XMStoreFloat3(this->scale);
}
