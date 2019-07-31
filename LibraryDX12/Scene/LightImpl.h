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

    void setPower(float pos) override;
    float getPower() const override;

	LightImpl();

protected:
    XMFLOAT3 position;
    float power;
    XMFLOAT3 color;
};