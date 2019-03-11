#include "DXD/Window.h"

class WindowImpl : public Window {
public:
    WindowImpl(int width, int height);
    void printSomething() override;

protected:
    int width;
    int height;
};
