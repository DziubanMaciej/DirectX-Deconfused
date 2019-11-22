#include "DXD/DXD.h"

struct Handler : DXD::CallbackHandler {
    DXD::Object *object = {};
    void onUpdate(unsigned int deltaTimeMicroseconds) {
        static float rotation{};
        rotation += 0.000001f * deltaTimeMicroseconds;
        object->setRotation(XMFLOAT3{0, 1, 1}, rotation);
    }
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hprev, LPSTR cmdline, int show) {
    auto application = DXD::Application::create(true);

    auto mesh = DXD::Mesh::createFromObj(*application, L"Resources/meshes/cube_normals.obj", false, false, true);

    auto cube = DXD::Object::create(*mesh);
    cube->setPosition(0, 0, 0);
    cube->setColor(0.2f, 0.2f, 0.2f);

    auto wall = DXD::Object::create(*mesh);
    wall->setPosition(0, 0, 5);
    wall->setColor(0, 0.7f, 0.7f);
    wall->setScale(200, 200, 0.01f);

    auto light = DXD::Light::create(DXD::Light::LightType::SPOT_LIGHT);
    light->setDirection(0, 0, 1);
    light->setPower(1.2f);
    light->setPosition(1, 1, -5);
    light->setColor(0.6f, 0.7f, 0.f);

    auto camera = DXD::Camera::create();
    camera->setEyePosition(0, 0, -5);
    camera->setFocusPoint(0, 0, 0);

    auto handler = Handler{};
    handler.object = cube.get();

    auto scene = DXD::Scene::create();
    scene->setBackgroundColor(0, 0.2f, 0.2f);
    scene->addObject(*cube);
    scene->addObject(*wall);
    scene->addLight(*light);
    scene->setCamera(*camera);

    auto window = DXD::Window::create(*application, L"myWindow", L"myWindow", hInstance, 800, 600);
    window->setScene(*scene);
    application->setCallbackHandler(&handler);
    window->show();
    window->messageLoop();

    return 0;
}
