#include "DXD/Window.h"

#include <iostream>

int main() {
    auto window = Window::createWindow(10, 20);
    window->printSomething();
    std::cin.get();
}
