#define _USE_MATH_DEFINES
#include "Common/FpsCounter.h"

#include <DXD/DXD.h>
#include <algorithm>
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
        prepObjects();
        prepLights();
        prepCamera();
        prepPostProcesses();
        prepSprites();
        prepText();
        prepScene();
        updateCamera();
        DXD::log("Preparing done! Some resources may still be loaded asynchronously\n");

        window->setScene(*scene);
        application->setCallbackHandler(this);
    }
    void run() {
        window->show();
        window->messageLoop();
    }

private:
    void prepMeshes() {
        DXD::log("Loading meshes...\n");

        struct MeshCreationData {
            std::string name;
            std::wstring filePath;
            bool loadTextures;
            bool computeTangents;
        };

        MeshCreationData meshesCreationData[] = {
            {"cubeNormalMesh", L"Resources/meshes/cube_normals.obj", false, false},
            {"cubeNormalUv", L"Resources/meshes/cube_normals_uvs.obj", true, false},
            {"cubeNormalUvTangents", L"Resources/meshes/cube_normals_uvs.obj", true, true},
            {"teapot", L"Resources/meshes/teapot_normals.obj", false, false},
            {"dennisMesh", L"Resources/meshes/dennis.obj", true, false},
            {"intelMesh", L"Resources/meshes/corei7.obj", false, false},
            {"aventadorMesh", L"Resources/meshes/aventador.obj", false, false},
            {"porsheMesh", L"Resources/meshes/porshe.obj", true, false}};

        for (const auto &data : meshesCreationData) {
            const bool asynchronousLoading = true;
            meshes[data.name] = DXD::Mesh::createFromObj(*application, data.filePath, data.loadTextures,
                                                         data.computeTangents, asynchronousLoading);
            assert(meshes[data.name]);
        }
    }
    void prepObjects() {
        DXD::log("Configuring objects...\n");

        std::unordered_map<std::string, std::string> meshObjectMap = {
            {"leftWall", "cubeNormalMesh"},
            {"rightWall", "cubeNormalMesh"},
            {"glowingCube1", "cubeNormalMesh"},
            {"glowingCube2", "cubeNormalMesh"},
            {"glowingCube3", "cubeNormalMesh"},
            {"glowingCube4", "cubeNormalMesh"},
            {"wood", "cubeNormalUv"},
            {"glowingBlue", "cubeNormalMesh"},
            {"glowingRed", "cubeNormalMesh"},
            {"stand", "cubeNormalMesh"},
            {"ground", "cubeNormalUv"},
            {"aventador1", "aventadorMesh"},
            {"porshe1", "porsheMesh"},
            {"aventador2", "aventadorMesh"},
            {"porshe2", "porsheMesh"},
            {"dennis", "dennisMesh"}};

        for (auto object : meshObjectMap) {
            objects.insert({object.first, DXD::Object::create(*meshes[object.second])});
        }

        objects["leftWall"]->setPosition(-9, 0, 0);
        objects["leftWall"]->setScale(0.5, 2, 10);
        objects["leftWall"]->setColor(0.2f, 0.2f, 0.2f);

        objects["rightWall"]->setPosition(9, 0, 0);
        objects["rightWall"]->setScale(0.5, 2, 10);
        objects["rightWall"]->setColor(0.2f, 0.2f, 0.2f);

        objects["glowingCube1"]->setPosition(-9.0f, 0.0f, 3.0f);
        objects["glowingCube1"]->setBloomFactor(1.0f);
        objects["glowingCube1"]->setColor(0.9f, 0.9f, 0.9f);
        objects["glowingCube1"]->setScale(0.75, 1.5f, 0.5f);

        objects["glowingCube2"]->setPosition(9.0f, 0.0f, 3.0f);
        objects["glowingCube2"]->setBloomFactor(1.0f);
        objects["glowingCube2"]->setColor(0.9f, 0.9f, 0.9f);
        objects["glowingCube2"]->setScale(0.75, 1.5f, 0.5f);

        objects["glowingCube3"]->setPosition(-9.0f, 0.0f, -3.0f);
        objects["glowingCube3"]->setBloomFactor(1.0f);
        objects["glowingCube3"]->setColor(0.9f, 0.9f, 0.9f);
        objects["glowingCube3"]->setScale(0.75, 1.5f, 0.5f);

        objects["glowingCube4"]->setPosition(9.0f, 0.0f, -3.0f);
        objects["glowingCube4"]->setBloomFactor(1.0f);
        objects["glowingCube4"]->setColor(0.9f, 0.9f, 0.9f);
        objects["glowingCube4"]->setScale(0.75, 1.5f, 0.5f);

        objects["glowingBlue"]->setPosition(-8.5f, 1.0f, -9.0f);
        objects["glowingBlue"]->setBloomFactor(1.0f);
        objects["glowingBlue"]->setColor(0.0f, 0.0f, 0.9f);
        objects["glowingBlue"]->setScale(0.25f, 0.25f, 0.25f);

        objects["glowingRed"]->setPosition(8.5f, 1.0f, -9.0f);
        objects["glowingRed"]->setBloomFactor(1.0f);
        objects["glowingRed"]->setColor(0.9f, 0.0f, 0.0f);
        objects["glowingRed"]->setScale(0.25f, 0.25f, 0.25f);

        objects["stand"]->setPosition(0, -2.5, 0);
        objects["stand"]->setSpecularity(0.2f);
        objects["stand"]->setScale(12, 0.5, 12);

        objects["wood"]->setPosition(0, -2.4f, 0);
        objects["wood"]->setScale(1.5f, 0.5f, 11);
        objects["wood"]->setTexture(textures["wood"].get());
        objects["wood"]->setTextureScale(3, 22);

        objects["ground"]->setTexture(textures["grass"].get());
        objects["ground"]->setPosition(0, -3.5, 0);
        objects["ground"]->setSpecularity(0.0f);
        objects["ground"]->setScale(100, 1.0f, 100);
        objects["ground"]->setTextureScale(50, 50);

        objects["porshe1"]->setPosition(4.0f, -1.475f, 2.0f);
        objects["porshe1"]->setSpecularity(0.6f);
        objects["porshe1"]->setScale(0.9f, 0.9f, 0.9f);
        objects["porshe1"]->setTexture(textures["porsche"].get());
        objects["porshe1"]->setRotation(XMFLOAT3(0, 1, 0), float(M_PI / 4));

        objects["aventador1"]->setPosition(-4.0f, -2.0f, 2.0f);
        objects["aventador1"]->setColor(0.0f, 0.0f, 0.0f);
        objects["aventador1"]->setSpecularity(0.6f);
        objects["aventador1"]->setScale(0.9f, 0.9f, 0.9f);
        objects["aventador1"]->setRotation(XMFLOAT3(0, 1, 0), float(-M_PI / 4));

        objects["porshe2"]->setPosition(-4.0f, -1.475f, -2.0f);
        objects["porshe2"]->setSpecularity(0.6f);
        objects["porshe2"]->setScale(0.9f, 0.9f, 0.9f);
        objects["porshe2"]->setTexture(textures["porsche"].get());
        objects["porshe2"]->setRotation(XMFLOAT3(0, 1, 0), float(M_PI / 4));

        objects["aventador2"]->setPosition(4.0f, -2.0f, -2.0f);
        objects["aventador2"]->setColor(0.0f, 0.0f, 0.0f);
        objects["aventador2"]->setSpecularity(0.6f);
        objects["aventador2"]->setScale(0.9f, 0.9f, 0.9f);
        objects["aventador2"]->setRotation(XMFLOAT3(0, 1, 0), float(-M_PI / 4));

        objects["dennis"]->setPosition(-2, -2, -8);
        objects["dennis"]->setScale(0.01f, 0.01f, 0.01f);
        objects["dennis"]->setRotation(XMFLOAT3(0, 1, 0), 90);
        objects["dennis"]->setTexture(textures["dennis"].get());
    }
    void prepLights() {
        DXD::log("Loading lights...\n");

        struct LightCreationData {
            std::string name;
            DXD::Light::LightType type;
        };

        std::vector<LightCreationData> lightNames = {
            {"sunLight", DXD::Light::LightType::DIRECTIONAL_LIGHT},
            {"redLight", DXD::Light::LightType::SPOT_LIGHT},
            {"blueLight", DXD::Light::LightType::SPOT_LIGHT}};

        for (const auto &data : lightNames) {
            lights.insert({data.name, DXD::Light::create(data.type)});
        }

        lights["sunLight"]->setColor(1.0f, 1.0f, 1.0f);
        lights["sunLight"]->setPosition(-12, 12, 6);
        lights["sunLight"]->setDirection(1, -1, -0.5);
        lights["sunLight"]->setPower(12);

        lights["redLight"]->setColor(1.0f, 0.0f, 0.0f);
        lights["redLight"]->setPosition(8.5f, 1.0f, -9.0f);
        lights["redLight"]->setDirection(-1, 0, 1);
        lights["redLight"]->setPower(2);

        lights["blueLight"]->setColor(0.0f, 0.0f, 1.0f);
        lights["blueLight"]->setPosition(-8.5f, 1.0f, -9.0f);
        lights["blueLight"]->setDirection(1, 0, 1);
        lights["blueLight"]->setPower(2);
    }
    void prepTextures() {
        DXD::log("Loading textures...\n");

        struct TextureCreationData {
            std::string name;
            std::wstring path;
            bool asynchronousLoading;
        };

        TextureCreationData texturesCreationData[] = {
            {"brickwall", L"Resources/textures/brickwall.jpg", true},
            {"brickwall_normal", L"Resources/textures/brickwall_normal.jpg", false},
            {"porsche", L"Resources/textures/porsche.bmp", true},
            {"dennis", L"Resources/textures/dennis.jpg", true},
            {"wood", L"Resources/textures/wood.jpg", true},
            {"grass", L"Resources/textures/grass.jpg", true}};

        for (const auto &data : texturesCreationData) {
            textures[data.name] = DXD::Texture::createFromFile(*application, data.path, data.asynchronousLoading);
            assert(textures[data.name]);
        }
    }
    void prepCamera() {
        DXD::log("Preparing camera...\n");
        camera = DXD::Camera::create();
        camera->setUpDirection(0, 1, 0);
        camera->setFovAngleYDeg(70);
        camera->setNearZ(0.1f);
        camera->setFarZ(140.0f);
    }
    void prepPostProcesses() {
        DXD::log("Loading post processes...\n");

        postProcesses["blackBars"] = DXD::PostProcess::create();
        postProcesses["blackBars"]->setBlackBars(0.0f, 0.0f, 0.05f, 0.05f);
        postProcesses["blackBars"]->setEnabled(false);

        postProcesses["gaussianBlur"] = DXD::PostProcess::create();
        postProcesses["gaussianBlur"]->setGaussianBlur(gaussianBlurPassCount, 5);
        postProcesses["gaussianBlur"]->setEnabled(false);

        postProcesses["sepia"] = DXD::PostProcess::create();
        postProcesses["sepia"]->setLinearColorCorrectionSepia();
        postProcesses["sepia"]->setEnabled(false);
    }
    void prepSprites() {
        DXD::log("Loading sprites...\n");

        struct SpriteTextureCreationData {
            std::string name;
            std::wstring path;
        };

        struct SpriteCreationData {
            std::string name;
            std::string textureName;
            int spriteSizeX;
            int spriteOffsetX;
            int spriteSizeY;
            int spriteOffsetY;
            DXD::Sprite::HorizontalAlignment horizontalAlignment;
            DXD::Sprite::VerticalAlignment verticalAlignment;
        };
        SpriteTextureCreationData spriteTexturesCreationData[] = {
            {"shrek", L"Resources/sprites/shrek.png"},
            {"rectangle-alpha", L"Resources/sprites/rectangle-alpha.png"},
            {"crosshair", L"Resources/sprites/crosshair.png"}};

        SpriteCreationData spritesCreationData[] = {
            {"rectangle-alpha", "rectangle-alpha", 165, 0, 33, 0, DXD::Sprite::HorizontalAlignment::LEFT, DXD::Sprite::VerticalAlignment::TOP}};

        for (const auto &data : spriteTexturesCreationData) {
            spriteTextures[data.name] = DXD::Texture::createFromFile(*application, data.path, false);
            assert(spriteTextures[data.name]);
        }
        for (const auto &data : spritesCreationData) {
            sprites[data.name] = DXD::Sprite::create(*spriteTextures[data.textureName], data.spriteSizeX, data.spriteOffsetX, data.spriteSizeY, data.spriteOffsetY, data.horizontalAlignment, data.verticalAlignment);
            assert(sprites[data.name]);
        }
    }

    void prepText() {
        DXD::log("Loading texts...\n");
        std::vector<std::string> textNames = {
            "fpsCounter"};

        for (auto text : textNames) {
            texts.insert({text, DXD::Text::create()});
        }

        texts["fpsCounter"]->setAlignment(DXDTextHorizontalAlignment::LEFT, DXDTextVerticalAlignment::TOP);
        texts["fpsCounter"]->setFontStyle(DXDFontStyle::NORMAL);
        texts["fpsCounter"]->setFontWeight(DXDFontWeight::ULTRA_BLACK);
        texts["fpsCounter"]->setColor(0, 0, 0, 1);
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
            scene->addPostProcess(*postProcess.second);
        }
        for (auto &sprite : sprites) {
            scene->addSprite(*sprite.second);
        }
        for (auto &text : texts) {
            scene->addText(*text.second);
        }
    }

    // Internal game logic
    void updateCamera() {
        auto camera = scene->getCamera();
        camera->setEyePosition(cameraPosition);
        camera->setLookDirection(focusDirection);
    }
    void sceneMoveTick(unsigned int deltaTimeMicroseconds) {
        static float rotation = 0.f;
        rotation += 0.000001f * deltaTimeMicroseconds;
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

            //DXD::log("AngleX = %f\n", angleX);
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
        case '3':
            application->getSettings().setShadowsQuality(application->getSettings().getShadowsQuality() - 1);
            break;
        case '4':
            application->getSettings().setShadowsQuality(application->getSettings().getShadowsQuality() + 1);
            break;
        case '5':
            application->getSettings().setSsaoEnabled(!application->getSettings().getSsaoEnabled());
            break;
        case '6':
            application->getSettings().setSsrEnabled(!application->getSettings().getSsrEnabled());
            break;
        case '7':
            postProcesses["blackBars"]->setEnabled(!postProcesses["blackBars"]->isEnabled());
            break;
        case '8':
            postProcesses["sepia"]->setEnabled(!postProcesses["sepia"]->isEnabled());
            break;
        case '0':
            gaussianBlurPassCount = std::min(gaussianBlurPassCount + 1, 5u);
            postProcesses["gaussianBlur"]->setGaussianBlur(gaussianBlurPassCount, 5);
            break;
        case '9':
            if (gaussianBlurPassCount > 0u) {
                gaussianBlurPassCount--;
            }
            postProcesses["gaussianBlur"]->setGaussianBlur(gaussianBlurPassCount, 5);
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

    // Constants
    constexpr static float movementSpeed = 0.7f;
    constexpr static float cameraRotationSpeed = 0.007f;

    // Data
    unsigned int lastMouseX, lastMouseY;
    bool lookingAroundEnabled = false;
    bool fullscreen = false;
    bool toggleSceneMovement = true;
    float angleX = 0.f;
    float angleY = 0.f;
    XMFLOAT3 cameraPosition{0, 4, -20};
    XMFLOAT3 focusDirection{0, -0.4f, 1};
    FpsCounter<180> fpsCounter;
    UINT gaussianBlurPassCount = 0u;

    // General DXD objects
    std::unique_ptr<DXD::Application> application;
    std::unique_ptr<DXD::Window> window;
    std::unique_ptr<DXD::Camera> camera;
    std::unique_ptr<DXD::Scene> scene;

    // DXD Scene elements
    std::unordered_map<std::string, std::unique_ptr<DXD::Mesh>> meshes;
    std::unordered_map<std::string, std::unique_ptr<DXD::Light>> lights;
    std::unordered_map<std::string, std::unique_ptr<DXD::Object>> objects;
    std::unordered_map<std::string, std::unique_ptr<DXD::Text>> texts;
    std::unordered_map<std::string, std::unique_ptr<DXD::PostProcess>> postProcesses;
    std::unordered_map<std::string, std::unique_ptr<DXD::Texture>> textures;
    std::unordered_map<std::string, std::unique_ptr<DXD::Texture>> spriteTextures;
    std::unordered_map<std::string, std::unique_ptr<DXD::Sprite>> sprites;
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hprev, LPSTR cmdline, int show) {
    Game game{hInstance};
    game.run();
    return 0;
}
