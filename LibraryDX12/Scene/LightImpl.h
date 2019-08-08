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

	LightImpl();

    XMMATRIX smViewProjectionMatrix;

protected:
    XMFLOAT3 position;
    float power;
    XMFLOAT3 color;
    XMFLOAT3 direction;
};
