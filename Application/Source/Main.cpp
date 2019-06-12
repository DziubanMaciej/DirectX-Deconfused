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

struct MyCallbackHandler : DXD::CallbackHandler {
    MyCallbackHandler(DXD::Window &window, DXD::Object &object, DXD::Scene &scene) : window(window), object(object), scene(scene) {
        auto camera = scene.getCamera();
        XMFLOAT4 tmpEyePos;
        XMStoreFloat4(&tmpEyePos, camera->getEyePosition());
        cameraX = tmpEyePos.x;
        cameraY = tmpEyePos.y;
        cameraRadius = tmpEyePos.z;
    }
    void onResize(int newWidth, int newHeight) override {
        DXD::log("Resized to: %d, %d\n", newWidth, newHeight);
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
            window.setFullscreen(fullscreen);
        }
        if (vkCode == 'A') {
            static float rotation = 0.f;
            rotation += 0.1f;
            object.setRotation(rotation, rotation, rotation);
        }
    }

private:
    void updateCamera() {
        auto camera = scene.getCamera();
        float x = cameraRadius * cosf((cameraX) / 10.f);
        float y = cameraRadius * cosf((cameraY) / 10.f);
        float z = cameraRadius * sinf((cameraX) / 10.f); // to jest zle, ale dobrze wyglada
        camera->setEyePosition(x, y, z);
        scene.setCamera(*camera);
    }
    bool fullscreen = false;
    bool lButtonHold = false;
    unsigned int dxPos, dyPos;
    float cameraRadius;
    int cameraX, cameraY;
    DXD::Window &window;
    DXD::Object &object;
    DXD::Scene &scene;
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hprev, LPSTR cmdline, int show) {
    auto application = DXD::Application::create(true);
    auto window = DXD::Window::create(*application, L"myClass", L"myWindow", hInstance, 300, 300);

    auto teapotMesh = DXD::Mesh::create(*application);
    if (teapotMesh->loadFromObj("Resources/meshes/teapot.obj") != 0)
        return -1;

    auto texture = DXD::Texture::createFromFile(*application, "Resources/wood.jpg");

    auto object1 = DXD::Object::create();
    object1->setMesh(*teapotMesh);
    object1->setPosition(0, 6, 0);

    auto object2 = DXD::Object::create();
    object2->setMesh(*teapotMesh);
    object2->setPosition(0, 0, 0);

    auto object3 = DXD::Object::create();
    object3->setMesh(*teapotMesh);
    object3->setPosition(0, -6, 0);

    auto camera = DXD::Camera::create();
    camera->setEyePosition(0, 0, -20);
    camera->setFocusPoint(0, 0, 0);
    camera->setUpDirection(0, 1, 0);
    camera->setFovAngleYDeg(45);
    camera->setNearZ(0.1f);
    camera->setFarZ(100.0f);

    auto scene = DXD::Scene::create();
    scene->addObject(*object1);
    scene->addObject(*object2);
    scene->addObject(*object3);
    scene->setBackgroundColor(0.7f, 0.4f, 0.2f);
    scene->setCamera(*camera);
    window->setScene(*scene);

    MyCallbackHandler handler{*window, *object2, *scene};
    application->setCallbackHandler(&handler);

    window->show();
    window->messageLoop();
    return 0;
}
