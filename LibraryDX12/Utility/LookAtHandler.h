#pragma once

#include <DXD/ExternalHeadersWrappers/DirectXMath.h>

class LookAtHandler {
public:
    void setPosition(float x, float y, float z) {
        this->position = XMVectorSet(x, y, z, 1.f);
        this->otherRepresentationDirty = true;
    }

    XMVECTOR getPosition() const {
        return this->position;
    }

    void setFocusPoint(float x, float y, float z) {
        this->focusPoint = XMVectorSet(x, y, z, 1.f);
        this->usingFocusPoint = true;
        this->otherRepresentationDirty = true;
    }
    XMVECTOR getFocusPoint() {
        if (!this->usingFocusPoint && this->otherRepresentationDirty) {
            this->focusPoint = XMVectorAdd(this->position, this->lookDirection);
            this->otherRepresentationDirty = false;
        }
        return this->focusPoint;
    }

    void setLookDirection(float x, float y, float z) {
        this->lookDirection = XMVectorSet(x, y, z, 0.f);
        this->usingFocusPoint = false;
        this->otherRepresentationDirty = true;
    };

    XMVECTOR getLookDirection() {
        if (this->usingFocusPoint && this->otherRepresentationDirty) {
            this->lookDirection = XMVector3Normalize(XMVectorSubtract(this->focusPoint, this->position));
            this->otherRepresentationDirty = false;
        }
        return this->lookDirection;
    }

private:
    XMVECTOR position = {0, 0, 0};
    XMVECTOR focusPoint = {0, 0, 1};
    XMVECTOR lookDirection = {0, 0, 1};
    bool usingFocusPoint = false;
    bool otherRepresentationDirty = false;
};
