#include "LightImpl.h"

namespace DXD {
std::unique_ptr<Light> Light::create() {
    return std::unique_ptr<Light>{new LightImpl()};
}
} // namespace DXD

LightImpl::LightImpl() {
    position = XMFLOAT3(0, 0, 0);
    color = XMFLOAT3(0, 0, 0);
    direction = XMFLOAT3(0.001f, -1.0f, 0.001);
    power = 1;
    type = DXD::LightType::SPOT_LIGHT;
}

void LightImpl::setPosition(FLOAT x, FLOAT y, FLOAT z) {
    setPosition(XMFLOAT3{x, y, z});
}

void LightImpl::setPosition(XMFLOAT3 pos) {
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
    setDirection(XMFLOAT3{x + 0.001f, y, z + 0.001f});
}

void LightImpl::setDirection(XMFLOAT3 xyz) {
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
