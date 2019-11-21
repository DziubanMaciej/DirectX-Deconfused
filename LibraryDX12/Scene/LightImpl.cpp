#include "LightImpl.h"

#include "Utility/ThrowIfFailed.h"

namespace DXD {
std::unique_ptr<Light> Light::create(LightType type) {
    return std::unique_ptr<Light>{new LightImpl(type)};
}
} // namespace DXD

LightImpl::LightImpl(LightType type)
    : type(type) {
}

// ------------------------------------------------------------- Position and rotation in 3D space

void LightImpl::setPosition(FLOAT x, FLOAT y, FLOAT z) {
    this->lookAtHandler.setPosition(x, y, z);
    this->dirtyView = true;
}

void LightImpl::setPosition(XMFLOAT3 pos) {
    this->lookAtHandler.setPosition(pos.x, pos.y, pos.z);
    this->dirtyView = true;
}

XMFLOAT3 LightImpl::getPosition() const {
    return XMStoreFloat3(this->lookAtHandler.getPosition());
}

void LightImpl::setDirection(FLOAT x, FLOAT y, FLOAT z) {
    this->dirtyView = true;
    this->lookAtHandler.setLookDirection(x, y, z);
}

void LightImpl::setDirection(XMFLOAT3 xyz) {
    this->dirtyView = true;
    this->lookAtHandler.setLookDirection(xyz.x, xyz.y, xyz.z);
}

XMFLOAT3 LightImpl::getDirection() const {
    return XMStoreFloat3(this->lookAtHandler.getLookDirection());
}

void LightImpl::setFocusPoint(float x, float y, float z) {
    this->dirtyView = true;
    this->lookAtHandler.setFocusPoint(x, y, z);
}

void LightImpl::setFocusPoint(XMFLOAT3 vec) {
    this->dirtyView = true;
    this->lookAtHandler.setFocusPoint(vec.x, vec.y, vec.z);
}

XMFLOAT3 LightImpl::getFocusPoint() const {
    return XMStoreFloat3(this->lookAtHandler.getFocusPoint());
}

// ------------------------------------------------------------- Light-specific fields

void LightImpl::setColor(FLOAT r, FLOAT g, FLOAT b) {
    setColor(XMFLOAT3{r, g, b});
}

void LightImpl::setColor(XMFLOAT3 rgb) {
    this->color = rgb;
}

XMFLOAT3 LightImpl::getColor() const {
    return color;
}

void LightImpl::setPower(float power) {
    this->power = power;
}

float LightImpl::getPower() const {
    return power;
}

// ------------------------------------------------------------- Matrices calculation

XMMATRIX LightImpl::getShadowMapViewMatrix() {
    if (this->dirtyView) {
        const XMVECTOR lightPosition = lookAtHandler.getPosition();
        const XMVECTOR lightFocusPoint = lookAtHandler.getFocusPoint();
        this->smViewMatrix = XMMatrixLookAtLH(lightPosition, lightFocusPoint, XMVectorSet(0, 1, 0, 0));
        this->dirtyView = false;
    }
    return this->smViewMatrix;
}

XMMATRIX LightImpl::getShadowMapProjectionMatrix() {
    if (this->dirtyProj) {
        switch (this->type) {
        case LightType::SPOT_LIGHT:
            this->smProjectionMatrix = XMMatrixPerspectiveFovLH(90, 1, 0.1f, 160.0f);
            break;
        case LightType::DIRECTIONAL_LIGHT:
            this->smProjectionMatrix = XMMatrixOrthographicLH(40, 40, 0.1f, 160.0f);
            break;
        default:
            UNREACHABLE_CODE();
        }
        this->dirtyProj = false;
    }
    return this->smProjectionMatrix;
}

XMMATRIX LightImpl::getShadowMapViewProjectionMatrix() {
    return XMMatrixMultiply(getShadowMapViewMatrix(), getShadowMapProjectionMatrix());
}
