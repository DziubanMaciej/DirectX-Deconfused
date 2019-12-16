#pragma once

#include "DXD/Light.h"

#include "Utility/LookAtHandler.h"

class LightImpl : public DXD::Light {
protected:
    friend class DXD::Light;
    LightImpl(LightType type);

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

    void setFocusPoint(float x, float y, float z) override;
    void setFocusPoint(XMFLOAT3 vec) override;
    XMFLOAT3 getFocusPoint() const override;

    void setPower(float pos) override;
    float getPower() const override;

    void setType(LightType type) { this->type = type; }
    LightType getType() { return type; }

    XMMATRIX getShadowMapViewMatrix();
    XMMATRIX getShadowMapProjectionMatrix();
    XMMATRIX getShadowMapViewProjectionMatrix();
    bool isViewOrProjectionDirty() const;

protected:
    LightType type;
    mutable LookAtHandler lookAtHandler;
    XMFLOAT3 color = { 1,1,1 };
    float power = 1.f;

    XMMATRIX smViewMatrix = {};
    XMMATRIX smProjectionMatrix = {};
    XMMATRIX smViewProjectionMatrix = {};

    bool dirtyView = true;
    bool dirtyProj = true;
};
