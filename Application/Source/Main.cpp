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
#include "DXD/Settings.h"
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
            {"cube", {L"Resources/meshes/cube.obj", false}},
            {"cubeNormal", {L"Resources/meshes/cube_normals.obj", false}},
            {"smallCube", {L"Resources/meshes/small_cube.obj", false}},
            {"flat", {L"Resources/meshes/flat_normals.obj", false}},
            {"extraFlat", {L"Resources/meshes/extra_flat_normals.obj", false}},
            //{"actor", {L"Resources/meshes/dennis.obj", false}},
            {"dxd", {L"Resources/meshes/dxd_comicsans.obj", false}},
            {"car", {L"Resources/meshes/porshe.obj", true}}};
        for (auto mesh : meshMap) {
            const auto creationData = mesh.second;
            meshes.insert({mesh.first, DXD::Mesh::createFromObj(*application, creationData.filePath, creationData.useTextures)});
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
                {"flatMesh1", "flat"},
                {"extraFlatMesh1", "extraFlat"},
                {"smallCubeMesh1", "smallCube"},
                {"smallCubeMesh2", "smallCube"},
                {"smallCubeMesh3", "smallCube"},
                {"carMesh1", "car"},
                //{"actorMesh1", "actor"},
                {"dxdMesh1", "dxd"}};

            for (auto object : meshObjectMap) {
                objects.insert({object.first, DXD::Object::create(*meshes[object.second])});
            }

            objects["teapotMesh1"]->setPosition(8, -1, 0);
            objects["teapotMesh1"]->setScale(0.1f, 0.1f, 0.1f);
            objects["teapotMesh1"]->setColor(0.0f, 1.0f, 0.0f);

            objects["teapotMesh2"]->setPosition(0, 1, 0);
            objects["teapotMesh2"]->setScale(0.1f, 0.1f, 0.1f);

            objects["teapotMesh3"]->setPosition(-8, -1, 0);
            objects["teapotMesh3"]->setScale(0.1f, 0.1f, 0.1f);
            objects["teapotMesh3"]->setColor(0.0f, 0.0f, 1.0f);

            objects["cubeNormalMesh1"]->setPosition(-8, -1, -6);

            objects["cubeNormalMesh2"]->setPosition(9, -1, -9);

            objects["flatMesh1"]->setPosition(0, -3, 0);
            objects["flatMesh1"]->setSpecularity(0.5f);

            objects["extraFlatMesh1"]->setPosition(0, -5, 0);
            objects["extraFlatMesh1"]->setSpecularity(0);

            objects["smallCubeMesh1"]->setPosition(-12, 12, 6);

            objects["smallCubeMesh2"]->setPosition(7, 4, 6);

            objects["smallCubeMesh3"]->setPosition(12, 1, -12);

            objects["carMesh1"]->setPosition(0, -1, -4); // y -2 for aventador
            objects["carMesh1"]->setColor(0.1f, 0, 0.1f);
            objects["carMesh1"]->setSpecularity(3);
            objects["carMesh1"]->setScale(0.9f, 0.9f, 0.9f);
            objects["carMesh1"]->setTexture(porsheTexture.get());

            //objects["actorMesh1"]->setPosition(-2, -2, -8);
            //objects["actorMesh1"]->setScale(0.01f, 0.01f, 0.01f);

            objects["dxdMesh1"]->setPosition(0, 2, 15);
            objects["dxdMesh1"]->setScale(20.f, 20.f, 20.f);
            objects["dxdMesh1"]->setRotation({0, 1, 0}, static_cast<float>(M_PI));
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
        porsheTexture = DXD::Texture::createFromFile(*application, L"Resources/textures/porsche.bmp");
        assert(porsheTexture);
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
    void prepScene() {
        DXD::log("Preparing scene...\n");
        scene = DXD::Scene::create(*application);
        for (auto &object : objects) {
            scene->addObject(*object.second);
        }
        scene->setBackgroundColor(0.3f, 0.8f, 1.0f);
        scene->setAmbientLight(0.1f, 0.1f, 0.1f);
        for (auto &light : lights) {
            scene->addLight(*light.second);
        }
        scene->setCamera(*camera);
        DXD::log("Done!\n");
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

        objects["carMesh1"]->setRotation(XMFLOAT3(0, 1, 0), rotation - 45); // +180 for aventador
        objects["carMesh1"]->setPosition(7 * sinf(rotation) * 0.6f, -1.475, 7 * cosf(rotation) * 0.6f);
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
    std::unordered_map<std::string, std::unique_ptr<DXD::Mesh>> meshes;
    std::unordered_map<std::string, std::unique_ptr<DXD::Light>> lights;
    std::unordered_map<std::string, std::unique_ptr<DXD::Object>> objects;

    // TODO: unordered_map for three below
    std::unique_ptr<DXD::Texture> porsheTexture;
    std::unique_ptr<DXD::Scene> scene;
    std::unique_ptr<DXD::Camera> camera;
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hprev, LPSTR cmdline, int show) {
    Game game{hInstance};
    game.run();
    return 0;
}
