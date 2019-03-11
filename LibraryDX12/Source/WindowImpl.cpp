#include <iostream>
#include "WindowImpl.h"

std::unique_ptr<Window> Window::createWindow(int width, int height) {
    return std::unique_ptr<Window>{new WindowImpl(width, height)};
}

WindowImpl::WindowImpl(int width, int height) : width(width), height(height) {
}

void WindowImpl::printSomething() {
    std::cout << "Hello world from dll! width=" << width << ", height=" << height << '\n';
}
