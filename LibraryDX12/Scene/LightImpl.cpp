#include "LightImpl.h"

namespace DXD {
std::unique_ptr<Light> Light::create() {
    return std::unique_ptr<Light>{new LightImpl()};
}
} // namespace DXD

LightImpl::LightImpl(){
    position = XMFLOAT3(0, 0, 0);
    color = XMFLOAT3(0, 0, 0);
    power = 1;
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

XMFLOAT3 LightImpl::getColor() const {
    return color;
}

void LightImpl::setPower(float power) {
    this->power = power;
}

float LightImpl::getPower() const {
    return power;
}
