#include "DXD/Application.h"
#include "DXD/CallbackHandler.h"
#include "DXD/Mesh.h"
#include "DXD/Object.h"
#include "DXD/Scene.h"
#include "DXD/Logger.h"
#include "DXD/Window.h"
#include <iostream>
#include <string>

struct MyCallbackHandler : DXD::CallbackHandler {
    MyCallbackHandler(DXD::Window &window) : window(window) {}

    void onResize(int newWidth, int newHeight) override {
        DXD::log("Resized to: %d, %d\n", newWidth, newHeight);
    }
    void onKeyDown(unsigned int vkCode) override {
        if (vkCode == 'F') {
            fullscreen = !fullscreen;
            window.setFullscreen(fullscreen);
        }
    }

private:
    bool fullscreen = false;
    DXD::Window &window;
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hprev, LPSTR cmdline, int show) {
    auto application = DXD::Application::create(true);
    auto window = DXD::Window::create(*application, L"myClass", L"myWindow", hInstance, 300, 300);

    MyCallbackHandler handler{*window};
    application->setCallbackHandler(&handler);

    auto my_mesh = DXD::Mesh::create(*application);
    if (my_mesh->loadFromObj("Resources/meshes/teapot.obj") != 0)
        return -1;

    auto my_object = DXD::Object::create();
    my_object->setMesh(*my_mesh);
    my_object->setPosition(0, 6, 9);

    auto scene = DXD::Scene::create();
    scene->addObject(*my_object);
    scene->setBackgroundColor(0.7f, 0.4f, 0.2f);
    window->setScene(*scene);

    window->show();
    window->messageLoop();
    return 0;
}
