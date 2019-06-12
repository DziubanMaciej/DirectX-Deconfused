#include "DXD/Application.h"
#include "DXD/CallbackHandler.h"
#include "DXD/Camera.h"
#include "DXD/Logger.h"
#include "DXD/Mesh.h"
#include "DXD/Object.h"
#include "DXD/Scene.h"
#include "DXD/Texture.h"
#include "DXD/Window.h"

#include <iostream>
#include <string>
#include <vector>

struct Game : DXD::CallbackHandler {
    Game(HINSTANCE hInstance) {
        application = DXD::Application::create(true);
        window = DXD::Window::create(*application, L"myClass", L"myWindow", hInstance, 300, 300);
        teapotMesh = DXD::Mesh::create(*application);
        assert(teapotMesh->loadFromObj("Resources/meshes/teapot.obj") == 0);
        woodTexture = DXD::Texture::createFromFile(*application, "Resources/wood.jpg");

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*teapotMesh);
        objects.back()->setPosition(0, 6, 0);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*teapotMesh);
        objects.back()->setPosition(0, 0, 0);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*teapotMesh);
        objects.back()->setPosition(0, -6, 0);

        camera = DXD::Camera::create();
        camera->setEyePosition(0, 0, -20);
        camera->setFocusPoint(0, 0, 0);
        camera->setUpDirection(0, 1, 0);
        camera->setFovAngleYDeg(45);
        camera->setNearZ(0.1f);
        camera->setFarZ(100.0f);

        const XMFLOAT3 eyePos = XMStoreFloat3(camera->getEyePosition());
        cameraX = eyePos.x;
        cameraY = eyePos.y;
        cameraRadius = eyePos.z;

        scene = DXD::Scene::create();
        scene->addObject(*objects[0]);
        scene->addObject(*objects[1]);
        scene->addObject(*objects[2]);
        scene->setBackgroundColor(0.7f, 0.4f, 0.2f);
        scene->setCamera(*camera);
        window->setScene(*scene);

        application->setCallbackHandler(this);
    }

    void run() {
        window->show();
        window->messageLoop();
    }

    void onMouseWheel(int zDelta) override {
        float zoomFactor = 2.5f;
        cameraRadius += zDelta > 0 ? zoomFactor : -zoomFactor;
        if (cameraRadius == 0.f)
            cameraRadius += .1f;
        updateCamera();
    }
    void onLButtonDown(unsigned int xPos, unsigned int yPos) override {
        dxPos = xPos;
        dyPos = yPos;
        lButtonHold = true;
        DXD::log("LButtonDown\n");
    }
    void onLButtonUp(unsigned int xPos, unsigned int yPos) override {
        lButtonHold = false;
        DXD::log("LButtonUp\n");
    }
    void onMouseMove(unsigned int xPos, unsigned int yPos) override {
        if (lButtonHold) {
            cameraX += dxPos - xPos;
            cameraY += dyPos - yPos;
            updateCamera();
            dxPos = xPos;
            dyPos = yPos;
        }
    }
    void onKeyDown(unsigned int vkCode) override {
        if (vkCode == 'F') {
            fullscreen = !fullscreen;
            window->setFullscreen(fullscreen);
        }
        if (vkCode == 'A') {
            static float rotation = 0.f;
            rotation += 0.1f;
            objects[1]->setRotation(rotation, rotation, rotation);
        }
    }

private:
    void updateCamera() {
        auto camera = scene->getCamera();
        float x = cameraRadius * cosf((cameraX) / 10.f);
        float y = cameraRadius * cosf((cameraY) / 10.f);
        float z = cameraRadius * sinf((cameraX) / 10.f); // to jest zle, ale dobrze wyglada
        camera->setEyePosition(x, y, z);
    }

    bool fullscreen = false;
    bool lButtonHold = false;
    unsigned int dxPos, dyPos;
    float cameraRadius;
    int cameraX, cameraY;

    std::unique_ptr<DXD::Application> application;
    std::unique_ptr<DXD::Window> window;
    std::unique_ptr<DXD::Mesh> teapotMesh;
    std::vector<std::unique_ptr<DXD::Object>> objects;
    std::unique_ptr<DXD::Texture> woodTexture;
    std::unique_ptr<DXD::Scene> scene;
    std::unique_ptr<DXD::Camera> camera;
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hprev, LPSTR cmdline, int show) {
    Game game{hInstance};
    game.run();
    return 0;
}
