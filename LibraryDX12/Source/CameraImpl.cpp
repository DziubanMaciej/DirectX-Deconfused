#include "CameraImpl.h"

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
}

void CameraImpl::setFocusPoint(float x, float y, float z) {
    this->focusPoint = XMVectorSet(x, y, z, 1.f);
}

void CameraImpl::setUpDirection(float x, float y, float z) {
    this->upDirection = XMVectorSet(x, y, z, 0.f);
}

void CameraImpl::setFovAngleYDeg(float val) {
    this->fovAngleY = XMConvertToRadians(val);
}

float CameraImpl::getFovAngleYDeg() {
    return XMConvertToRadians(fovAngleY);
}

XMMATRIX CameraImpl::getViewMatrix() {
    return XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
}

XMMATRIX CameraImpl::getProjectionMatrix() {
    return XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);
}
