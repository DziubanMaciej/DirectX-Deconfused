#include "DXD/Window.h"

#include <iostream>

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hprev, LPSTR cmdline, int show) {
    auto window = Window::create(L"asd", L"XDD", hInstance, 300, 300);
    window->show();
    window->messageLoop();
    return 0;
}
