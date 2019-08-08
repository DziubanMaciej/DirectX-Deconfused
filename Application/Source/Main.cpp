#define _USE_MATH_DEFINES
#include "FpsCounter.h"

#include "DXD/Application.h"
#include "DXD/CallbackHandler.h"
#include "DXD/Camera.h"
#include "DXD/Light.h"
#include "DXD/Logger.h"
#include "DXD/Mesh.h"
#include "DXD/Object.h"
#include "DXD/Scene.h"
#include "DXD/Texture.h"
#include "DXD/Window.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

struct Game : DXD::CallbackHandler {
    Game(HINSTANCE hInstance) {
        application = DXD::Application::create(true);
        window = DXD::Window::create(*application, L"myClass", L"myWindow", hInstance, 1240, 720);
        DXD::log("Loading meshes...\n");
        teapotMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/teapot_normals.obj");
        assert(teapotMesh);
        cubeMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/cube.obj");
        assert(cubeMesh);
        cubeNormalMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/cube_normals.obj");
        assert(cubeNormalMesh);
        smallCubeMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/small_cube.obj");
        assert(smallCubeMesh);
        flatMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/flat_normals.obj");
        assert(flatMesh);
        extraFlatMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/extra_flat_normals.obj");
        assert(extraFlatMesh);
        carMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/porshe.obj");
        assert(carMesh);
        //actorMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/dennis.obj");
        //assert(actorMesh);
        dxdMesh = DXD::Mesh::createFromObj(*application, "Resources/meshes/dxd_comicsans.obj");
        assert(dxdMesh);
        DXD::log("Done!\n");
        woodTexture = DXD::Texture::createFromFile(*application, "Resources/wood.jpg");
        assert(woodTexture);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*teapotMesh);
        objects.back()->setPosition(8, -1, 0);
        objects.back()->setScale(0.1f, 0.1f, 0.1f);
        objects.back()->setColor(0.0f, 1.0f, 0.0f);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*teapotMesh);
        objects.back()->setPosition(0, 1, 0);
        objects.back()->setScale(0.1f, 0.1f, 0.1f);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*teapotMesh);
        objects.back()->setPosition(-8, -1, 0);
        objects.back()->setScale(0.1f, 0.1f, 0.1f);
        objects.back()->setColor(0.0f, 0.0f, 1.0f);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*cubeNormalMesh);
        objects.back()->setPosition(-8, -1, -6);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*cubeNormalMesh);
        objects.back()->setPosition(9, -1, -9);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*flatMesh);
        objects.back()->setPosition(0, -3, 0);
        objects.back()->setSpecularity(0.5f);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*extraFlatMesh);
        objects.back()->setPosition(0, -5, 0);
        objects.back()->setSpecularity(0);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*smallCubeMesh);
        objects.back()->setPosition(0, 4, 0);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*carMesh);
        objects.back()->setPosition(0, -1, -4); // y -2 for aventador
        objects.back()->setColor(0, 0, 0);
        objects.back()->setSpecularity(3);
        objects.back()->setScale(0.9f, 0.9f, 0.9f);

        sunLight = DXD::Light::create();
        sunLight->setColor(1.0f, 1.0f, 1.0f);
        sunLight->setPosition(-12, 12, 0);
        //sunLight->setDirection(1, -1, 0);
        sunLight->setPower(10);

        moonLight = DXD::Light::create();
        moonLight->setColor(0.0f, 1.0f, 1.0f);
        moonLight->setPosition(7, 4, 6);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*smallCubeMesh);
        objects.back()->setPosition(7, 4, 6);

        redLight = DXD::Light::create();
        redLight->setColor(1.0f, 0.0f, 0.0f);
        redLight->setPosition(0, 1, -8);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*smallCubeMesh);
        objects.back()->setPosition(0, 1, -8);

        blueLight = DXD::Light::create();
        blueLight->setColor(0.0f, 0.0f, 1.0f);
        blueLight->setPosition(0, 1, -8);

        //objects.push_back(DXD::Object::create());
        //objects.back()->setMesh(*actorMesh);
        //objects.back()->setPosition(-2, -2, -8);
        //objects.back()->setScale(0.01f, 0.01f, 0.01f);

        objects.push_back(DXD::Object::create());
        objects.back()->setMesh(*dxdMesh);
        objects.back()->setPosition(0, 2, 15);
        objects.back()->setScale(20.f, 20.f, 20.f);
        objects.back()->setRotation({0, 1, 0}, static_cast<float>(M_PI));

        scene = DXD::Scene::create(*application);
        scene->addObject(*objects[0]);
        scene->addObject(*objects[1]);
        scene->addObject(*objects[2]);
        scene->addObject(*objects[3]);
        scene->addObject(*objects[4]);
        scene->addObject(*objects[5]);
        scene->addObject(*objects[6]);
        scene->addObject(*objects[7]);
        scene->addObject(*objects[8]);
        scene->addObject(*objects[9]);
        scene->addObject(*objects[10]);
        scene->addObject(*objects[11]);
        scene->setBackgroundColor(0.3f, 0.8f, 1.0f);
        scene->setAmbientLight(0.1f, 0.1f, 0.1f);
        scene->addLight(*sunLight);
        scene->addLight(*moonLight);
        scene->addLight(*redLight);
        scene->addLight(*blueLight);

        camera = DXD::Camera::create();
        camera->setUpDirection(0, 1, 0);
        camera->setFovAngleYDeg(70);
        camera->setNearZ(0.1f);
        camera->setFarZ(140.0f);
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
        case 'T':
            toggleSceneMovement ^= 1;
        case VK_SPACE:
            if (toggleSceneMovement)
                break;
            sceneMoveTick(10000);
            return;
        }
    }

    void onUpdate(unsigned int deltaTimeMicroseconds) override {
        fpsCounter.push(deltaTimeMicroseconds);
        if (fpsCounter.getFrameIndex() % 512) {
            DXD::log("Frame time=%d us, FPS=%d\n", deltaTimeMicroseconds, fpsCounter.getFps());
        }

        if (!toggleSceneMovement)
            return;
        sceneMoveTick(deltaTimeMicroseconds);
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
    void sceneMoveTick(unsigned int deltaTimeMicroseconds) {
        static float rotation = 0.f;
        rotation += 0.000001f * deltaTimeMicroseconds;
        XMFLOAT3 sunPos = sunLight->getPosition();
        sunPos.x = 7 * sin(-rotation / 4);
        sunPos.z = 7 * cos(-rotation / 4);
        sunLight->setPosition(sunPos);

        moonLight->setPower((sin(rotation) + 1) / 2);

        blueLight->setDirection(sin(rotation), 0, cos(rotation));
        redLight->setDirection(-sin(rotation), 0, -cos(rotation));

        objects[8]->setRotation(XMFLOAT3(0, 1, 0), rotation - 45); // +180 for aventador
        objects[8]->setPosition(7 * sinf(rotation) * 0.6f, -1.475, 7 * cosf(rotation) * 0.6f);

        objects[7]->setPosition(sunPos);
        objects[1]->setRotation(rotation, rotation, rotation);
    }
    constexpr static float movementSpeed = 0.7f;
    constexpr static float cameraRotationSpeed = 0.007f;

    unsigned int lastMouseX, lastMouseY;
    bool lookingAroundEnabled = false;
    bool fullscreen = false;
    bool toggleSceneMovement = true;
    float angleX = 0.f;
    float angleY = 0.f;
    XMFLOAT3 cameraPosition{0, 4, -20};
    XMFLOAT3 focusDirection{0, -0.2f, 1};
    FpsCounter<180> fpsCounter;

    std::unique_ptr<DXD::Application> application;
    std::unique_ptr<DXD::Window> window;
    std::unique_ptr<DXD::Mesh> teapotMesh;
    std::unique_ptr<DXD::Mesh> cubeMesh;
    std::unique_ptr<DXD::Mesh> smallCubeMesh;
    std::unique_ptr<DXD::Mesh> cubeNormalMesh;
    std::unique_ptr<DXD::Mesh> flatMesh;
    std::unique_ptr<DXD::Mesh> extraFlatMesh;
    std::unique_ptr<DXD::Mesh> carMesh;
    std::unique_ptr<DXD::Mesh> actorMesh;
    std::unique_ptr<DXD::Mesh> dxdMesh;
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
