#include "CameraImpl.h"

namespace DXD {
std::unique_ptr<Camera> Camera::create() {
    return std::unique_ptr<Camera>{new CameraImpl()};
}
} // namespace DXD

// ------------------------------------------------------------- Position and rotation in 3D space

void CameraImpl::setEyePosition(float x, float y, float z) {
    this->eyePosition = XMVectorSet(x, y, z, 1.f);
    this->dirtyView = true;
}

void CameraImpl::setEyePosition(XMFLOAT3 vec) {
    this->dirtyView = true;
    this->eyePosition = XMLoadFloat3(&vec);
}

XMFLOAT3 CameraImpl::getEyePosition() const {
    return XMStoreFloat3(this->eyePosition);
}

void CameraImpl::setFocusPoint(float x, float y, float z) {
    this->dirtyView = true;
    this->focusPoint = XMVectorSet(x, y, z, 1.f);
}

void CameraImpl::setFocusPoint(XMFLOAT3 vec) {
    this->dirtyView = true;
    this->focusPoint = XMLoadFloat3(&vec);
}

XMFLOAT3 CameraImpl::getFocusPoint() const {
    return XMStoreFloat3(this->focusPoint);
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
        viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
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

XMMATRIX CameraImpl::getInvViewMatrix() {
    if (dirtyView) {
        dirtyView = false;
        viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
    }
    return XMMatrixInverse(nullptr, viewMatrix);
}

XMMATRIX CameraImpl::getInvProjectionMatrix() {
    if (dirtyProj) {
        dirtyProj = false;
        projectionMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);
    }
    return XMMatrixInverse(nullptr, projectionMatrix);
}