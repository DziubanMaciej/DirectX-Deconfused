#pragma once

#include "DXD/Camera.h"

class CameraImpl : public DXD::Camera {
protected:
    friend class DXD::Camera;
    CameraImpl();
    ~CameraImpl();

public:
    void setEyePosition(float x, float y, float z) override;
    void setEyePosition(XMFLOAT3 vec) override;
    XMFLOAT3 getEyePosition() const override;

    void setFocusPoint(float x, float y, float z) override;
    void setFocusPoint(XMFLOAT3 vec) override;
    XMFLOAT3 getFocusPoint() const override;

    void setUpDirection(float x, float y, float z) override;
    void setUpDirection(XMFLOAT3 vec) override;
    XMFLOAT3 getUpDirection() const override;

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
