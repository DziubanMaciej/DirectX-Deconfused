#pragma once

#include "Resource/TextureImpl.h"
#include "Scene/MeshImpl.h"

#include "DXD/Mesh.h"
#include "DXD/Object.h"

class ObjectImpl : public DXD::Object {
protected:
    friend class DXD::Object;
    ObjectImpl(DXD::Mesh &mesh);

public:
    MeshImpl &getMesh() { return mesh; }
    const MeshImpl &getMesh() const { return mesh; }
    const XMMATRIX &getModelMatrix();

    void setPosition(FLOAT x, FLOAT y, FLOAT z) override;
    void setPosition(XMFLOAT3 pos) override;
    XMFLOAT3 getPosition() const override;

    void setRotation(XMFLOAT3 axis, float angle) override;
    void setRotation(float roll, float yaw, float pitch) override;
    XMFLOAT3 getRotationQuaternion() const override;

    void setRotationOrigin(FLOAT x, FLOAT y, FLOAT z) override;
    void setRotationOrigin(XMFLOAT3 pos) override;
    XMFLOAT3 getRotationOrigin() const override;

    void setScale(FLOAT x, FLOAT y, FLOAT z) override;
    void setScale(XMFLOAT3 scale) override;
    XMFLOAT3 getScale() const override;

    void setColor(FLOAT r, FLOAT g, FLOAT b) override;
    XMFLOAT3 getColor() const override;

    void setSpecularity(float s) override;
    float getSpecularity() const override;

    void setBloomFactor(float bloomFactor) override;
    float getBloomFactor() const override;

    void setTexture(DXD::Texture *texture) override { this->texture = static_cast<TextureImpl *>(texture); }
    DXD::Texture *getTexture() override { return texture; }
    TextureImpl *getTextureImpl() { return texture; }

    void setNormalMap(DXD::Texture *normalMap) override { this->normalMap = static_cast<TextureImpl *>(normalMap); }
    DXD::Texture *getNormalMap() override { return normalMap; }
    TextureImpl *getNormalMapImpl() { return normalMap; }

    void setTextureScale(float u, float v) override;
    void setTextureScale(XMFLOAT2 uv) override;
    XMFLOAT2 getTextureScale() const override;

    bool isReady();

protected:
    MeshImpl &mesh;
    TextureImpl *texture = {};
    TextureImpl *normalMap = {};
    XMFLOAT2 textureScale = {1, 1};

    XMVECTOR scale = {1, 1, 1};
    XMVECTOR position = {0, 0, 0};
    XMVECTOR rotationQuaternion = XMQuaternionIdentity();
    XMVECTOR rotationOrigin = {0, 0, 0};

    XMFLOAT3 color = {0, 0, 0};
    float specularity = 0.0f;
    float bloomFactor = 0.f;

    XMMATRIX modelMatrix;
    bool modelMatrixDirty = true;
};
