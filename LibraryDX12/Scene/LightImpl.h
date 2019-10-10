#pragma once

#include "DXD/Light.h"

class LightImpl : public DXD::Light {
protected:
    friend class DXD::Light;

public:
    void setPosition(FLOAT x, FLOAT y, FLOAT z) override;
    void setPosition(XMFLOAT3 pos) override;
    XMFLOAT3 getPosition() const override;

    void setColor(FLOAT r, FLOAT g, FLOAT b) override;
    void setColor(XMFLOAT3 rgb) override;
    XMFLOAT3 getColor() const override;

    void setDirection(FLOAT x, FLOAT y, FLOAT z) override;
    void setDirection(XMFLOAT3 xyz) override;
    XMFLOAT3 getDirection() const override;

    void setPower(float pos) override;
    float getPower() const override;

    void setType(DXD::LightType type) { this->type = type; }
    DXD::LightType getType() { return type; }

    LightImpl();

    XMMATRIX getShadowMapViewMatrix();
    XMMATRIX getShadowMapProjectionMatrix();
    XMMATRIX getShadowMapViewProjectionMatrix();

protected:
    XMFLOAT3 position;
    float power;
    XMFLOAT3 color;
    XMFLOAT3 direction;
    DXD::LightType type;

    XMMATRIX smViewMatrix = {};
    XMMATRIX smProjectionMatrix = {};
    XMMATRIX smViewProjectionMatrix = {};

    bool dirtyView = true;
    bool dirtyProj = true;
};
