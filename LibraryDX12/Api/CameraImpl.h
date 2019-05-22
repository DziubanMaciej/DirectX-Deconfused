#pragma once

#include "DXD/Camera.h"

class CameraImpl : public DXD::Camera {
protected:
    friend class DXD::Camera;
    CameraImpl();
    ~CameraImpl();

public:
    void setEyePosition(float x, float y, float z) override;
    void setEyePosition(XMVECTOR vec) override;
    XMVECTOR getEyePosition() const override { return eyePosition; }

    void setFocusPoint(float x, float y, float z) override;
    void setFocusPoint(XMVECTOR vec) override;
    XMVECTOR getFocusPoint() const override { return focusPoint; }

    void setUpDirection(float x, float y, float z) override;
    void setUpDirection(XMVECTOR vec) override;
    XMVECTOR getUpDirection() const override { return upDirection; }

    void setFovAngleY(float val) override;
    void setFovAngleYDeg(float val) override;
    float getFovAngleY() const override { return fovAngleY; }
    float getFovAngleYDeg() const override;

    void setAspectRatio(float val);
    float getAspectRatio() { return aspectRatio; }

    void setNearZ(float val) override;
    float getNearZ() const override { return nearZ; }

    void setFarZ(float val) override;
    float getFarZ() const override { return farZ; }

    XMMATRIX getViewMatrix();
    XMMATRIX getProjectionMatrix();

protected:
    XMVECTOR eyePosition;
    XMVECTOR focusPoint;
    XMVECTOR upDirection;
    float fovAngleY;
    float aspectRatio;
    float nearZ;
    float farZ;
    XMMATRIX viewMatrix;
    XMMATRIX projectionMatrix;
    bool dirtyView;
    bool dirtyProj;
};
