#include "LightImpl.h"

namespace DXD {
std::unique_ptr<Light> Light::create() {
    return std::unique_ptr<Light>{new LightImpl()};
}
} // namespace DXD

void LightImpl::setPosition(FLOAT x, FLOAT y, FLOAT z) {
    setPosition(XMFLOAT3{x, y, z});
}

void LightImpl::setPosition(XMFLOAT3 pos) {
    this->position = XMLoadFloat3(&pos);
}

XMFLOAT3 LightImpl::getPosition() const {
    return XMStoreFloat3(this->position);
}

void LightImpl::setColor(FLOAT r, FLOAT g, FLOAT b) {
    setColor(XMFLOAT3{r, g, b});
}

void LightImpl::setColor(XMFLOAT3 rgb) {
    this->color = XMLoadFloat3(&rgb);
}

XMFLOAT3 LightImpl::getColor() const {
    return XMStoreFloat3(this->color);
}
