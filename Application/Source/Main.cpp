#include "DXD/Application.h"
#include "DXD/CallbackHandler.h"
#include "DXD/Window.h"
#include "DXD/Scene.h"

#include <iostream>
#include <string>

struct MyCallbackHandler : DXD::CallbackHandler {
    void onResize(int newWidth, int newHeight) override {
        OutputDebugString((std::to_string(newWidth) + ", " + std::to_string(newHeight) + "\n").c_str());
    }
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hprev, LPSTR cmdline, int show) {
    MyCallbackHandler h;
    auto application = DXD::Application::create(true);
    application->setCallbackHandler(&h);

    auto window = DXD::Window::create(*application, L"myClass", L"myWindow", hInstance, 300, 300);
    auto scene = DXD::Scene::create();
    scene->setBackgroundColor(0.7f, 0.4f, 0.2f);
    window->setScene(*scene);

    window->show();
    window->messageLoop();
    return 0;
}
