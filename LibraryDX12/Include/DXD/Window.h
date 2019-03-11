#pragma once

#include "DXD/Export.h"

#include <memory>

class EXPORT Window {
protected:
    Window() = default;

public:
    static std::unique_ptr<Window> createWindow(int width, int height);

    virtual void printSomething() = 0;
};
