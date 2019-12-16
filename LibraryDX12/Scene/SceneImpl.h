#pragma once

#include "Resource/Resource.h"

#include <DXD/ExternalHeadersWrappers/d3d12.h>
#include <DXD/Scene.h>
#include <set>
#include <vector>

struct AlternatingResources;
class ApplicationImpl;
class CameraImpl;
class LightImpl;
class ObjectImpl;
class PostProcessImpl;
class RenderData;
class SpriteImpl;
class SwapChain;
class TextImpl;
class WindowImpl;

class SceneImpl : public DXD::Scene {
protected:
    friend class DXD::Scene;
    SceneImpl();

public:
    void render(SwapChain &swapChain, RenderData &renderData);

    void setBackgroundColor(float r, float g, float b) override;
    const FLOAT *getBackgroundColor() const { return backgroundColor; }

    void setAmbientLight(float r, float g, float b) override;
    const FLOAT *getAmbientLight() const { return ambientLight; }

    void setFogColor(float r, float g, float b) override;
    const FLOAT *getFogColor() const { return fogColor; }

    void setFogPower(float pow) override;
    float getFogPower() const { return fogPower; }

    void addLight(DXD::Light &light) override;
    unsigned int removeLight(DXD::Light &light) override;
    const auto &getLights() const { return lights; }

    void addPostProcess(DXD::PostProcess &postProcess) override;
    unsigned int removePostProcess(DXD::PostProcess &postProcess) override;
    const auto &getPostProcesses() const { return postProcesses; }

    void addObject(DXD::Object &object) override;
    unsigned int removeObject(DXD::Object &object) override;
    const auto &getObjects() const { return objects; }

    void addText(DXD::Text &text) override;
    unsigned int removeText(DXD::Text &text) override;
    const auto &getTexts() const { return texts; }

    void addSprite(DXD::Sprite &sprite) override;
    unsigned int removeSprite(DXD::Sprite &sprite) override;
    const auto &getSprites() const { return sprites; }

    void setCamera(DXD::Camera &camera) override;
    virtual DXD::Camera *getCamera() override;
    auto getCameraImpl() const { return camera; }

    Microsoft::WRL::ComPtr<ID3D12QueryHeap> queryHeap;
    std::unique_ptr<Resource> queryResult;

protected:
    void inspectObjectsNotReady();

    template <typename Type, typename TypeImpl>
    uint32_t removeFromScene(std::vector<TypeImpl *> &vector, Type &object) {
        TypeImpl *objectImpl = static_cast<TypeImpl *>(&object);
        const auto sizeBefore = vector.size();
        auto removeIterator = std::remove(vector.begin(), vector.end(), objectImpl);
        vector.erase(removeIterator, vector.end());
        return static_cast<uint32_t>(sizeBefore - vector.size());
    }

    // Data set by user
    FLOAT backgroundColor[3] = {};
    FLOAT ambientLight[3] = {};
    FLOAT fogColor[3] = {};
    FLOAT fogPower = 0;
    std::vector<LightImpl *> lights;
    std::set<ObjectImpl *> objects; // TODO might not be the best data structure for that
    std::set<ObjectImpl *> objectsNotReady;
    std::vector<TextImpl *> texts;
    std::vector<PostProcessImpl *> postProcesses = {};
    std::vector<SpriteImpl *> sprites = {};
    CameraImpl *camera;
};
