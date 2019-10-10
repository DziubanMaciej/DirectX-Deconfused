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

void LightImpl::setPosition(FLOAT x, FLOAT y, FLOAT z) {
    this->dirtyView = true;
    setPosition(XMFLOAT3{x, y, z});
}

void LightImpl::setPosition(XMFLOAT3 pos) {
    this->dirtyView = true;
    this->position = pos;
}

XMFLOAT3 LightImpl::getPosition() const {
    return position;
}

void LightImpl::setColor(FLOAT r, FLOAT g, FLOAT b) {
    setColor(XMFLOAT3{r, g, b});
}

void LightImpl::setColor(XMFLOAT3 rgb) {
    this->color = rgb;
}

XMFLOAT3 LightImpl::getDirection() const {
    return direction;
}

void LightImpl::setDirection(FLOAT x, FLOAT y, FLOAT z) {
    this->dirtyView = true;
    setDirection(XMFLOAT3{x + 0.001f, y, z + 0.001f});
}

void LightImpl::setDirection(XMFLOAT3 xyz) {
    this->dirtyView = true;
    this->direction = xyz;
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

XMMATRIX LightImpl::getShadowMapViewMatrix() {
    if (this->dirtyView) {
        const auto lightPosition = XMLoadFloat3(&position);
        const auto lightDirection = XMLoadFloat3(&direction);
        const auto lightFocusPoint = XMVectorAdd(lightDirection, lightPosition);
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
