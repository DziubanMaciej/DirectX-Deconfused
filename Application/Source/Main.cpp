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
        teapotMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/teapot.obj");
        assert(teapotMesh);
        woodTexture = DXD::Texture::createFromFile(*application, "Resources/wood.jpg");
        assert(woodTexture);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*teapotMesh);
        objects.back()->setPosition(0, 6, 0);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*teapotMesh);
        objects.back()->setPosition(0, 0, 0);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*teapotMesh);
        objects.back()->setPosition(0, -6, 0);

        scene = DXD::Scene::create();
        scene->addObject(*objects[0]);
        scene->addObject(*objects[1]);
        scene->addObject(*objects[2]);
        scene->setBackgroundColor(0.7f, 0.4f, 0.2f);

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
            cameraPosition.x -= movementSpeed;
            updateCamera();
            break;
        case 'D':
            cameraPosition.x += movementSpeed;
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
