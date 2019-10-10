#include "DXD/DXD.h"

struct Handler : DXD::CallbackHandler {
    DXD::Object *object = {};
    void onUpdate(unsigned int deltaTimeMicroseconds) {
        static float rotation{};
        rotation += 0.000001f * deltaTimeMicroseconds;
        object->setRotation(XMFLOAT3{0, 1, 0}, rotation);
    }
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hprev, LPSTR cmdline, int show) {
    auto application = DXD::Application::create(true);

    auto mesh = DXD::Mesh::createFromObj(*application, L"Resources/meshes/cube_normals.obj", false, false, true);

    auto object = DXD::Object::create(*mesh);
    object->setPosition(0, 0, 0);
    object->setColor(0.2f, 0.2f, 0.2f);

    auto light = DXD::Light::create();
    light->setType(DXD::LightType::SPOT_LIGHT);
    light->setDirection(0, 0, 1);
    light->setPower(0.2f);
    light->setPosition(0, 0, -5);
    light->setColor(0.6f, 0.7f, 0.f);

    auto camera = DXD::Camera::create();
    camera->setEyePosition(0, 0, -5);
    camera->setFocusPoint(0, 0, 0);

    auto handler = Handler{};
    handler.object = object.get();

    auto scene = DXD::Scene::create(*application);
    scene->setBackgroundColor(0, 0.2f, 0.2f);
    scene->addObject(*object);
    scene->addLight(*light);
    scene->setCamera(*camera);

    auto window = DXD::Window::create(*application, L"myWindow", L"myWindow", hInstance, 800, 600);
    window->setScene(*scene);
    application->setCallbackHandler(&handler);
    window->show();
    window->messageLoop();

    return 0;
}
