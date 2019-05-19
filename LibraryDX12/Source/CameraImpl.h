#pragma once

#include "DXD/Camera.h"

class CameraImpl : public DXD::Camera {
protected:
    friend class DXD::Camera;
    CameraImpl();
    ~CameraImpl();

public:
    void setEyePosition(float x, float y, float z);
    void setEyePosition(XMVECTOR vec);
    XMVECTOR getEyePosition() { return eyePosition; }

    void setFocusPoint(float x, float y, float z);
    void setFocusPoint(XMVECTOR vec);
    XMVECTOR getFocusPoint() { return focusPoint; }

    void setUpDirection(float x, float y, float z);
    void setUpDirection(XMVECTOR vec);
    XMVECTOR getUpDirection() { return upDirection; }

    void setFovAngleY(float val);
    void setFovAngleYDeg(float val);
    float getFovAngleY() { return fovAngleY; }
    float getFovAngleYDeg();

    void setAspectRatio(float val);
    float getAspectRatio() { return aspectRatio; }

    void setNearZ(float val);
    float getNearZ() { return nearZ; }

    void setFarZ(float val);
    float getFarZ() { return farZ; }

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
