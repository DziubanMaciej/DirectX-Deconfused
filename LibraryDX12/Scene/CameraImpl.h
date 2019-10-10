#pragma once

#include "DXD/Camera.h"

class CameraImpl : public DXD::Camera {
protected:
    friend class DXD::Camera;

public:
    void setEyePosition(float x, float y, float z) override;
    void setEyePosition(XMFLOAT3 vec) override;
    XMFLOAT3 getEyePosition() const override;

    void setFocusPoint(float x, float y, float z) override;
    void setFocusPoint(XMFLOAT3 vec) override;
    XMFLOAT3 getFocusPoint() const override;

    void setLookDirection(float x, float y, float z) override;
    void setLookDirection(XMFLOAT3 vec) override;
    XMFLOAT3 getLookDirection() const override;

    void setUpDirection(float x, float y, float z) override;
    void setUpDirection(XMFLOAT3 vec) override;
    XMFLOAT3 getUpDirection() const override;

    void setFovAngleY(float val) override;
    void setFovAngleYDeg(float val) override;
    float getFovAngleY() const override { return fovAngleY; }
    float getFovAngleYDeg() const override;

    void setNearZ(float val) override;
    float getNearZ() const override { return nearZ; }

    void setFarZ(float val) override;
    float getFarZ() const override { return farZ; }

    void setAspectRatio(float val);
    float getAspectRatio() { return aspectRatio; }

    XMVECTOR calculateFocusPoint() const;

    XMMATRIX getViewMatrix();
    XMMATRIX getProjectionMatrix();
    XMMATRIX getInvViewMatrix();
    XMMATRIX getInvProjectionMatrix();

protected:
    XMVECTOR eyePosition = {0, 0, 0};
    XMVECTOR focusPoint = {0, 0, 1};
    XMVECTOR lookDirection = {0, 0, 1};
    XMVECTOR upDirection = {0, 1, 0};
    float fovAngleY = XM_PI / 2;
    float aspectRatio = {};
    float nearZ = 0.0001f;
    float farZ = 1000.f;
    XMMATRIX viewMatrix;
    XMMATRIX projectionMatrix;
    bool dirtyView = false;
    bool dirtyProj = false;
    bool usingFocusPoint = false; // either focusPoint or lookDirection is used
};
