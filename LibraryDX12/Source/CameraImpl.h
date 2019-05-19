#pragma once

#include "DXD/Camera.h"

class CameraImpl : public DXD::Camera {
protected:
    friend class DXD::Camera;
    CameraImpl();
    ~CameraImpl();

public:
    void setEyePosition(float x, float y, float z);
    void setEyePosition(XMVECTOR vec) { this->eyePosition = vec; }
    XMVECTOR getEyePosition() { return eyePosition; }

    void setFocusPoint(float x, float y, float z);
    void setFocusPoint(XMVECTOR vec) { this->focusPoint = vec; }
    XMVECTOR getFocusPoint() { return focusPoint; }

    void setUpDirection(float x, float y, float z);
    void setUpDirection(XMVECTOR vec) { this->upDirection = vec; }
    XMVECTOR getUpDirection() { return upDirection; }

    void setFovAngleY(float val) { this->fovAngleY = val; }
    void setFovAngleYDeg(float val);
    float getFovAngleY() { return fovAngleY; }
    float getFovAngleYDeg();

    void setAspectRatio(float val) { this->aspectRatio = val; }
    float getAspectRatio() { return aspectRatio; }

    void setNearZ(float val) { this->nearZ = val; }
    float getNearZ() { return nearZ; }

    void setFarZ(float val) { this->farZ = val; }
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
    // TODO/PERF uzyc flagi dirty zeby nie generowac od nowa view i projection matrix za kazdym razem
};
