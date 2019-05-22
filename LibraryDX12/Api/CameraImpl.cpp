#include "Api/CameraImpl.h"

namespace DXD {
std::unique_ptr<Camera> Camera::create() {
    return std::unique_ptr<Camera>{new CameraImpl()};
}
} // namespace DXD

CameraImpl::CameraImpl() {
}

CameraImpl::~CameraImpl() {
}

void CameraImpl::setEyePosition(float x, float y, float z) {
    this->eyePosition = XMVectorSet(x, y, z, 1.f);
    this->dirtyView = true;
}

void CameraImpl::setEyePosition(XMVECTOR vec) {
    this->dirtyView = true;
    this->eyePosition = vec;
}

void CameraImpl::setFocusPoint(float x, float y, float z) {
    this->dirtyView = true;
    this->focusPoint = XMVectorSet(x, y, z, 1.f);
}

void CameraImpl::setFocusPoint(XMVECTOR vec) {
    this->dirtyView = true;
    this->focusPoint = vec;
}

void CameraImpl::setUpDirection(float x, float y, float z) {
    this->dirtyView = true;
    this->upDirection = XMVectorSet(x, y, z, 0.f);
}

void CameraImpl::setUpDirection(XMVECTOR vec) {
    this->dirtyView = true;
    this->upDirection = vec;
}

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

void CameraImpl::setAspectRatio(float val) {
    this->dirtyProj = true;
    this->aspectRatio = val;
}

void CameraImpl::setNearZ(float val) {
    this->dirtyProj = true;
    this->nearZ = val;
}

void CameraImpl::setFarZ(float val) {
    this->dirtyProj = true;
    this->farZ = val;
}
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
