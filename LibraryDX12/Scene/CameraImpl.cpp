#include "CameraImpl.h"

namespace DXD {
std::unique_ptr<Camera> Camera::create() {
    return std::unique_ptr<Camera>{new CameraImpl()};
}
} // namespace DXD

// ------------------------------------------------------------- Position and rotation in 3D space

void CameraImpl::setEyePosition(float x, float y, float z) {
    this->lookAtHandler.setPosition(x, y, z);
    this->dirtyView = true;
}

void CameraImpl::setEyePosition(XMFLOAT3 vec) {
    this->lookAtHandler.setPosition(vec.x, vec.y, vec.z);
    this->dirtyView = true;
}

XMFLOAT3 CameraImpl::getEyePosition() const {
    return XMStoreFloat3(this->lookAtHandler.getPosition());
}

void CameraImpl::setFocusPoint(float x, float y, float z) {
    this->lookAtHandler.setFocusPoint(x, y, z);
    this->dirtyView = true;
}

void CameraImpl::setFocusPoint(XMFLOAT3 vec) {
    this->lookAtHandler.setFocusPoint(vec.x, vec.y, vec.z);
    this->dirtyView = true;
}

XMFLOAT3 CameraImpl::getFocusPoint() const {
    return XMStoreFloat3(this->lookAtHandler.getFocusPoint());
}

void CameraImpl::setLookDirection(float x, float y, float z) {
    this->lookAtHandler.setLookDirection(x, y, z);
    this->dirtyView = true;
}

void CameraImpl::setLookDirection(XMFLOAT3 vec) {
    this->lookAtHandler.setLookDirection(vec.x, vec.y, vec.z);
    this->dirtyView = true;
}

XMFLOAT3 CameraImpl::getLookDirection() const {
    return XMStoreFloat3(this->lookAtHandler.getLookDirection());
}

void CameraImpl::setUpDirection(float x, float y, float z) {
    this->dirtyView = true;
    this->upDirection = XMVectorSet(x, y, z, 0.f);
}

void CameraImpl::setUpDirection(XMFLOAT3 vec) {
    this->dirtyView = true;
    this->upDirection = XMLoadFloat3(&vec);
}

XMFLOAT3 CameraImpl::getUpDirection() const {
    return XMStoreFloat3(this->upDirection);
}

// ------------------------------------------------------------- Other fields

void CameraImpl::setFovAngleY(float val) {
    this->dirtyProj = true;
    this->fovAngleY = val;
}

void CameraImpl::setFovAngleYDeg(float val) {
    this->dirtyProj = true;
    this->fovAngleY = XMConvertToRadians(val);
}

float CameraImpl::getFovAngleYDeg() const {
    return XMConvertToRadians(fovAngleY);
}

void CameraImpl::setNearZ(float val) {
    this->dirtyProj = true;
    this->nearZ = val;
}

void CameraImpl::setFarZ(float val) {
    this->dirtyProj = true;
    this->farZ = val;
}

// ------------------------------------------------------------- Internal fields

void CameraImpl::setAspectRatio(float val) {
    this->dirtyProj = true;
    this->aspectRatio = val;
}

// ------------------------------------------------------------- Transformation matrices getters

XMMATRIX CameraImpl::getViewMatrix() {
    if (dirtyView) {
        dirtyView = false;
        viewMatrix = XMMatrixLookAtLH(lookAtHandler.getPosition(), lookAtHandler.getFocusPoint(), upDirection);
    }
    return viewMatrix;
}

XMMATRIX CameraImpl::getProjectionMatrix() {
    if (dirtyProj) {
        dirtyProj = false;
        projectionMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);
    }
    return projectionMatrix;
}

XMMATRIX CameraImpl::getViewProjectionMatrix() {
    return XMMatrixMultiply(getViewMatrix(), getProjectionMatrix());
}

XMMATRIX CameraImpl::getInvViewMatrix() {
    return XMMatrixInverse(nullptr, getViewMatrix());
}

XMMATRIX CameraImpl::getInvProjectionMatrix() {
    return XMMatrixInverse(nullptr, getProjectionMatrix());
}
