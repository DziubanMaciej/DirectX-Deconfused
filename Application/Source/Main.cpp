﻿#include "DXD/Application.h"
#include "DXD/CallbackHandler.h"
#include "DXD/Camera.h"
#include "DXD/Light.h"
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
        window = DXD::Window::create(*application, L"myClass", L"myWindow", hInstance, 1240, 720);
        teapotMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/teapot.obj");
        assert(teapotMesh);
        cubeMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/cube.obj");
        assert(cubeMesh);
        cubeNormalMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/cube_normals.obj");
        assert(cubeNormalMesh);
        flatMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/flat_normals.obj");
        assert(flatMesh);
        extraFlatMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/extra_flat_normals.obj");
        assert(extraFlatMesh);
        woodTexture = DXD::Texture::createFromFile(*application, "Resources/wood.jpg");
        assert(woodTexture);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*teapotMesh);
        objects.back()->setPosition(8, -1, 0);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*teapotMesh);
        objects.back()->setPosition(0, 1, 0);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*teapotMesh);
        objects.back()->setPosition(-8, -1, 0);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*cubeNormalMesh);
        objects.back()->setPosition(-8, -1, -6);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*cubeNormalMesh);
        objects.back()->setPosition(9, -1, -9);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*flatMesh);
        objects.back()->setPosition(0, -3, 0);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*extraFlatMesh);
        objects.back()->setPosition(0, -5, 0);

		objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*cubeNormalMesh);
        objects.back()->setPosition(0, -1, 6);

        sunLight = DXD::Light::create();
        sunLight->setColor(1.0f, 1.0f, 0.0f);
        sunLight->setPosition(0, 4, 0);

        moonLight = DXD::Light::create();
        moonLight->setColor(0.0f, 1.0f, 1.0f);
        moonLight->setPosition(7, 4, 6);

        redLight = DXD::Light::create();
        redLight->setColor(1.0f, 0.0f, 0.0f);
        redLight->setPosition(-9, 2, -8);

        blueLight = DXD::Light::create();
        blueLight->setColor(0.0f, 0.0f, 1.0f);
        blueLight->setPosition(9, 2, -8);

        scene = DXD::Scene::create();
        scene->addObject(*objects[0]);
        scene->addObject(*objects[1]);
        scene->addObject(*objects[2]);
        scene->addObject(*objects[3]);
        scene->addObject(*objects[4]);
        scene->addObject(*objects[5]);
        scene->addObject(*objects[6]);
        scene->addObject(*objects[7]);
        scene->setBackgroundColor(0.1f, 0.1f, 0.3f);
        scene->addLight(*sunLight);
        scene->addLight(*moonLight);
        scene->addLight(*redLight);
        scene->addLight(*blueLight);

        camera = DXD::Camera::create();
        camera->setUpDirection(0, 1, 0);
        camera->setFovAngleYDeg(45);
        camera->setNearZ(0.1f);
        camera->setFarZ(100.0f);
        scene->setCamera(*camera);
        updateCamera();

        window->setScene(*scene);
        application->setCallbackHandler(this);
    }

    void run() {
        window->show();
        window->messageLoop();
    }

    void onMouseMove(unsigned int xPos, unsigned int yPos) override {
        if (lookingAroundEnabled) {
            XMFLOAT3 oldFocusDirection{0, 0, 1};

            // Update angles
            angleY += cameraRotationSpeed * (static_cast<int>(yPos) - static_cast<int>(lastMouseY));
            angleX -= cameraRotationSpeed * (static_cast<int>(xPos) - static_cast<int>(lastMouseX));

            // Rotate vertically
            const auto cosineY = cos(angleY);
            const auto sineY = sin(angleY);
            focusDirection.x = oldFocusDirection.x;
            focusDirection.y = oldFocusDirection.y * cosineY - oldFocusDirection.z * sineY;
            focusDirection.z = oldFocusDirection.y * sineY + oldFocusDirection.z * cosineY;

            // Rotate horizontally
            const auto cosineX = cos(angleX);
            const auto sineX = sin(angleX);
            oldFocusDirection = focusDirection;
            focusDirection.x = oldFocusDirection.x * cosineX - oldFocusDirection.z * sineX;
            focusDirection.y = oldFocusDirection.y;
            focusDirection.z = oldFocusDirection.x * sineX + oldFocusDirection.z * cosineX;

            updateCamera();

            DXD::log("AngleX = %f\n", angleX);
        }

        lastMouseX = xPos;
        lastMouseY = yPos;
    }

    void onKeyDown(unsigned int vkCode) override {
        switch (vkCode) {
        case 'F':
            fullscreen = !fullscreen;
            window->setFullscreen(fullscreen);
            break;
        case 'W':
            cameraPosition.x += focusDirection.x * movementSpeed;
            cameraPosition.y += focusDirection.y * movementSpeed;
            cameraPosition.z += focusDirection.z * movementSpeed;
            updateCamera();
            break;
        case 'S':
            cameraPosition.x -= focusDirection.x * movementSpeed;
            cameraPosition.y -= focusDirection.y * movementSpeed;
            cameraPosition.z -= focusDirection.z * movementSpeed;
            updateCamera();
            break;
        case 'A':
            cameraPosition.x += -focusDirection.z * movementSpeed;
            cameraPosition.z += focusDirection.x * movementSpeed;
            updateCamera();
            break;
        case 'D':
            cameraPosition.x -= -focusDirection.z * movementSpeed;
            cameraPosition.z -= focusDirection.x * movementSpeed;
            updateCamera();
            break;
        case 'Q':
            cameraPosition.y += movementSpeed;
            updateCamera();
            break;
        case 'E':
            cameraPosition.y -= movementSpeed;
            updateCamera();
            break;
        case 'L':
            lookingAroundEnabled = !lookingAroundEnabled;
            break;
        case VK_SPACE:
            static float rotation = 0.f;
            rotation += 0.1f;
            XMFLOAT3 sunPos = sunLight->getPosition();
            sunPos.x += 0.1f;
            if (sunPos.x >= 20) {
                sunPos.x = -20;
            }
            sunLight->setPosition(sunPos);

            objects[1]->setRotation(rotation, rotation, rotation);
            break;
        }
    }

private:
    void updateCamera() {
        auto camera = scene->getCamera();
        camera->setEyePosition(cameraPosition);

        XMFLOAT3 focusPoint{cameraPosition};
        focusPoint.x += focusDirection.x;
        focusPoint.y += focusDirection.y;
        focusPoint.z += focusDirection.z;
        camera->setFocusPoint(focusPoint);
    }

    constexpr static float movementSpeed = 0.7f;
    constexpr static float cameraRotationSpeed = 0.007f;

    unsigned int lastMouseX, lastMouseY;
    bool lookingAroundEnabled = false;
    bool fullscreen = false;
    float angleX = 0.f;
    float angleY = 0.f;
    XMFLOAT3 cameraPosition{0, 0, -20};
    XMFLOAT3 focusDirection{0, 0, 1};

    std::unique_ptr<DXD::Application> application;
    std::unique_ptr<DXD::Window> window;
    std::unique_ptr<DXD::Mesh> teapotMesh;
    std::unique_ptr<DXD::Mesh> cubeMesh;
    std::unique_ptr<DXD::Mesh> cubeNormalMesh;
    std::unique_ptr<DXD::Mesh> flatMesh;
    std::unique_ptr<DXD::Mesh> extraFlatMesh;
    std::unique_ptr<DXD::Light> sunLight;
    std::unique_ptr<DXD::Light> moonLight;
    std::unique_ptr<DXD::Light> redLight;
    std::unique_ptr<DXD::Light> blueLight;
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
