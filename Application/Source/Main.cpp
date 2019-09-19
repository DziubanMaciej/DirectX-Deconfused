﻿#define _USE_MATH_DEFINES
#include "FpsCounter.h"

#include "DXD/Application.h"
#include "DXD/CallbackHandler.h"
#include "DXD/Camera.h"
#include "DXD/Light.h"
#include "DXD/Logger.h"
#include "DXD/Mesh.h"
#include "DXD/Object.h"
#include "DXD/PostProcess.h"
#include "DXD/Scene.h"
#include "DXD/Settings.h"
#include "DXD/Text.h"
#include "DXD/Texture.h"
#include "DXD/Window.h"

#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class Game : DXD::CallbackHandler {
public:
    Game(HINSTANCE hInstance) {
        application = DXD::Application::create(true);
        window = DXD::Window::create(*application, L"myClass", L"myWindow", hInstance, 1240, 720);
        prepTextures();
        prepMeshes();
        prepLights();
        prepCamera();
        prepPostProcesses();
        prepText();
        prepScene();
        updateCamera();

        window->setScene(*scene);
        application->setCallbackHandler(this);
    }
    void run() {
        window->show();
        window->messageLoop();
    }

private:
    // preparing resources for the game
    void prepMeshes() {
        DXD::log("Loading meshes...\n");

        struct MeshCreationData {
            std::wstring filePath;
            bool useTextures;
        };

        std::unordered_map<std::string, MeshCreationData> meshMap = {
            {"teapot", {L"Resources/meshes/teapot_normals.obj", false}},
            {"cubeNormal", {L"Resources/meshes/cube_normals.obj", false}},
            {"actor", {L"Resources/meshes/dennis.obj", true}},
            {"dxd", {L"Resources/meshes/dxd_comicsans.obj", false}},
            {"intelMesh", {L"Resources/meshes/corei7.obj", false}},
            {"aventadorMesh", {L"Resources/meshes/aventador.obj", false}},
            {"porsheMesh", {L"Resources/meshes/porshe.obj", true}}};
        for (auto mesh : meshMap) {
            const auto creationData = mesh.second;
            meshes.insert({mesh.first, DXD::Mesh::createFromObj(*application, creationData.filePath, creationData.useTextures, true)});
            assert(meshes[mesh.first]);
        }

        // MESH OBJECTS
        {
            std::unordered_map<std::string, std::string> meshObjectMap = {
                {"teapotMesh1", "teapot"},
                {"teapotMesh2", "teapot"},
                {"teapotMesh3", "teapot"},
                {"cubeNormalMesh1", "cubeNormal"},
                {"cubeNormalMesh2", "cubeNormal"},
                {"cubeNormalMesh3", "cubeNormal"},
                {"flatMesh1", "cubeNormal"},
                {"intel", "intelMesh"},
                {"extraFlatMesh1", "cubeNormal"},
                {"aventador", "aventadorMesh"},
                {"porshe", "porsheMesh"},
                {"actorMesh1", "actor"}};

            for (auto object : meshObjectMap) {
                objects.insert({object.first, DXD::Object::create(*meshes[object.second])});
            }

            objects["teapotMesh1"]->setPosition(8, -1, 0);
            objects["teapotMesh1"]->setScale(0.1f, 0.1f, 0.1f);
            objects["teapotMesh1"]->setColor(0.0f, 0.8f, 0.0f);
            objects["teapotMesh1"]->setSpecularity(0.7f);

            objects["teapotMesh2"]->setPosition(0, 1, 0);
            objects["teapotMesh2"]->setScale(0.1f, 0.1f, 0.1f);
            objects["teapotMesh2"]->setColor(0.5f, 0.5f, 0.0f);
            objects["teapotMesh2"]->setSpecularity(0.7f);

            objects["teapotMesh3"]->setPosition(-8, -1, 0);
            objects["teapotMesh3"]->setScale(0.1f, 0.1f, 0.1f);
            objects["teapotMesh3"]->setColor(0.0f, 0.0f, 0.8f);
            objects["teapotMesh3"]->setSpecularity(0.7f);

            objects["cubeNormalMesh1"]->setPosition(-8, -1, -5.5);
            objects["cubeNormalMesh1"]->setScale(1, 1, 2);
            objects["cubeNormalMesh1"]->setSpecularity(1);

            objects["cubeNormalMesh2"]->setPosition(9, -1, -9);

            objects["cubeNormalMesh3"]->setPosition(0, -1.9, 0);
            objects["cubeNormalMesh3"]->setScale(2, 0.1f, 2);
            objects["cubeNormalMesh3"]->setSpecularity(1);

            objects["intel"]->setPosition(0, -1.9, -8);
            objects["intel"]->setScale(0.1f, 0.1f, 0.1f);

            objects["flatMesh1"]->setPosition(0, -2.5, 0);
            objects["flatMesh1"]->setSpecularity(0.2f);
            objects["flatMesh1"]->setScale(20, 0.5, 10);

            objects["extraFlatMesh1"]->setPosition(0, -4.0, 0);
            objects["extraFlatMesh1"]->setSpecularity(0.2f);
            objects["extraFlatMesh1"]->setScale(100, 1.0f, 100);
            objects["extraFlatMesh1"]->setColor(126.0f / 255.0f, 200.0f / 255.0f, 90.0f / 255.0f);

            objects["porshe"]->setPosition(0, -1, -4);
            objects["porshe"]->setSpecularity(0.8f);
            objects["porshe"]->setScale(0.9f, 0.9f, 0.9f);
            objects["porshe"]->setTexture(porsheTexture.get());

            objects["aventador"]->setPosition(-14, -2, 0);
            objects["aventador"]->setColor(0.0f, 113.0f / 255.0f, 197.0f/255.0f);
            objects["aventador"]->setSpecularity(0.8f);
            objects["aventador"]->setScale(0.9f, 0.9f, 0.9f);
            objects["aventador"]->setRotation(XMFLOAT3(0, 1, 0), -90);

            objects["actorMesh1"]->setPosition(-2, -2, -8);
            objects["actorMesh1"]->setScale(0.01f, 0.01f, 0.01f);
            objects["actorMesh1"]->setRotation(XMFLOAT3(0, 1, 0), 90);
            objects["actorMesh1"]->setSpecularity(0.02f);
            objects["actorMesh1"]->setColor(1, 224.0f / 255.0f, 189.0f / 255.0f);
            objects["actorMesh1"]->setTexture(dennisTexture.get());
        }
        DXD::log("Done!\n");
    }
    void prepLights() {
        DXD::log("Loading lights...\n");
        std::vector<std::string> lightNames = {
            "sunLight",
            "moonLight",
            "redLight",
            "blueLight"};
        for (auto light : lightNames) {
            lights.insert({light, DXD::Light::create()});
        }
        // LIGHT CONFIG
        {
            lights["sunLight"]->setColor(1.0f, 1.0f, 1.0f);
            lights["sunLight"]->setPosition(-12, 12, 6);
            lights["sunLight"]->setDirection(1, -1, -0.5);
            lights["sunLight"]->setPower(12);
            lights["sunLight"]->setType(DXD::LightType::DIRECTIONAL_LIGHT);

            lights["moonLight"]->setColor(0.0f, 1.0f, 1.0f);
            lights["moonLight"]->setPosition(7, 4, 6);
            lights["moonLight"]->setPower(2);

            lights["redLight"]->setColor(1.0f, 0.0f, 0.0f);
            lights["redLight"]->setPosition(-12, 1, -12);
            lights["redLight"]->setDirection(1, 0, 1);
            lights["redLight"]->setPower(2);

            lights["blueLight"]->setColor(0.0f, 0.0f, 1.0f);
            lights["blueLight"]->setPosition(12, 1, -12);
            lights["blueLight"]->setDirection(-1, 0, 1);
            lights["blueLight"]->setPower(2);
        }
        DXD::log("Done!\n");
    }
    void prepTextures() {
        porsheTexture = DXD::Texture::createFromFile(*application, L"Resources/textures/porsche.bmp", true);
        assert(porsheTexture);

        dennisTexture = DXD::Texture::createFromFile(*application, L"Resources/textures/dennis.jpg", true);
        assert(dennisTexture);
    }
    void prepCamera() {
        DXD::log("Preparing camera...\n");
        camera = DXD::Camera::create();
        camera->setUpDirection(0, 1, 0);
        camera->setFovAngleYDeg(70);
        camera->setNearZ(0.1f);
        camera->setFarZ(140.0f);
        DXD::log("Done!\n");
    }
    void prepPostProcesses() {
        //postProcesses.push_back(DXD::PostProcess::create());
        //postProcesses.back()->setBlackBars(0.0f, 0.0f, 0.05f, 0.05f);

        postProcesses.push_back(DXD::PostProcess::create());
        gaussianBlurPostProcess = postProcesses.back().get();
        gaussianBlurPostProcess->setGaussianBlur(gaussianBlurPassCount, 5);
    }

    void prepText() {
        DXD::log("Loading texts...\n");
        std::vector<std::string> textNames = {
            "fpsCounter"};

        for (auto text : textNames) {
            texts.insert({text, DXD::Text::create()});
        }

        texts["fpsCounter"]->setAlignment(DXDTextHorizontalAlignment::LEFT, DXDTextVerticalAlignment::TOP);
        texts["fpsCounter"]->setFontStyle(DXDFontStyle::ITALIC);
        texts["fpsCounter"]->setFontWeight(DXDFontWeight::ULTRA_BLACK);
        texts["fpsCounter"]->setFontSize(13.f);
    }

    void prepScene() {
        DXD::log("Preparing scene...\n");
        scene = DXD::Scene::create(*application);
        scene->setBackgroundColor(0.7f, 1.0f, 1.0f);
        scene->setAmbientLight(1.0f, 1.0f, 250.0f / 255.0f);
        scene->setCamera(*camera);

        for (auto &object : objects) {
            scene->addObject(*object.second);
        }
        for (auto &light : lights) {
            scene->addLight(*light.second);
        }
        for (auto &postProcess : postProcesses) {
            scene->addPostProcess(*postProcess);
        }
        for (auto &text : texts) {
            scene->addText(*text.second);
        }
        DXD::log("Preparing scene done.\n");
    }

    // Internal game logic
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

        objects["porshe"]->setRotation(XMFLOAT3(0, 1, 0), rotation - 45); // +180 for aventador
        objects["porshe"]->setPosition(7 * sinf(rotation) * 0.6f, -1.475f, 7 * cosf(rotation) * 0.6f);
        objects["teapotMesh2"]->setRotation(rotation, rotation, rotation);
    }

    // Callbacks
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
            break;
        case 'V':
            application->getSettings().setVerticalSyncEnabled(!application->getSettings().getVerticalSyncEnabled());
            break;
        case '0':
            gaussianBlurPassCount++;
            gaussianBlurPostProcess->setGaussianBlur(gaussianBlurPassCount, 5);
            break;
        case '9':
            gaussianBlurPassCount--;
            gaussianBlurPostProcess->setGaussianBlur(gaussianBlurPassCount, 5);
            break;
        }
    }
    void onUpdate(unsigned int deltaTimeMicroseconds) override {
        fpsCounter.push(deltaTimeMicroseconds);
        const bool useD2DForFpsCounter = true;
        if ((fpsCounter.getFrameIndex() % 60) == 0) {
            if (useD2DForFpsCounter) {
                std::wstring txt = L"FPS: ";
                txt.append(std::to_wstring(fpsCounter.getFps()));
                txt.append(L"\nFrame time [us]: ");
                txt.append(std::to_wstring(deltaTimeMicroseconds));
                texts["fpsCounter"]->setText(txt);
            } else {
                DXD::log("Frame time=%d us, FPS=%d\n", deltaTimeMicroseconds, fpsCounter.getFps());
            }
        }

        if (toggleSceneMovement) {
            sceneMoveTick(deltaTimeMicroseconds);
        }
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
    XMFLOAT3 focusDirection{0, -0.4f, 1};
    FpsCounter<180> fpsCounter;
    DXD::PostProcess *gaussianBlurPostProcess = nullptr;
    UINT gaussianBlurPassCount = 0u;

    std::unique_ptr<DXD::Application> application;
    std::unique_ptr<DXD::Window> window;
    std::unordered_map<std::string, std::unique_ptr<DXD::Mesh>> meshes;
    std::unordered_map<std::string, std::unique_ptr<DXD::Light>> lights;
    std::unordered_map<std::string, std::unique_ptr<DXD::Object>> objects;
    std::unordered_map<std::string, std::unique_ptr<DXD::Text>> texts;
    std::vector<std::unique_ptr<DXD::PostProcess>> postProcesses;

    // TODO: unordered_map for three below
    std::unique_ptr<DXD::Texture> porsheTexture;
    std::unique_ptr<DXD::Texture> dennisTexture;
    std::unique_ptr<DXD::Scene> scene;
    std::unique_ptr<DXD::Camera> camera;
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hprev, LPSTR cmdline, int show) {
    Game game{hInstance};
    game.run();
    return 0;
}
