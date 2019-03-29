#include "DXD/Application.h"
#include "DXD/CallbackHandler.h"
#include "DXD/Window.h"

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
    window->show();
    window->messageLoop();
    return 0;
}
