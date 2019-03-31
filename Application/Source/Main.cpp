#include "DXD/Application.h"
#include "DXD/CallbackHandler.h"
#include "DXD/Scene.h"
#include "DXD/Window.h"
#include <iostream>
#include <string>

struct MyCallbackHandler : DXD::CallbackHandler {
    MyCallbackHandler(DXD::Window &window) : window(window) {}

    void onResize(int newWidth, int newHeight) override {
        OutputDebugString((std::to_string(newWidth) + ", " + std::to_string(newHeight) + "\n").c_str());
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
    auto scene = DXD::Scene::create();

    MyCallbackHandler handler{*window};
    application->setCallbackHandler(&handler);
    scene->setBackgroundColor(0.7f, 0.4f, 0.2f);
    window->setScene(*scene);

    window->show();
    window->messageLoop();
    return 0;
}
