#pragma once

#include "DXD/Export.h"

namespace DXD {

class Window;

class EXPORT CallbackHandler {
public:
    virtual void onResize(int newWidth, int newHeight) {}
    virtual void onWindowShow(Window &window) {}
    virtual void onKeyDown(unsigned int vkCode) {}
    virtual void onKeyUp(unsigned int vkCode) {}
    virtual void onUpdate() {}
    virtual void onMouseWheel(int zDelta) {}
    virtual void onLButtonUp(unsigned int xPos, unsigned int yPos) {}
    virtual void onLButtonDown(unsigned int xPos, unsigned int yPos) {}
    virtual void onMouseMove(unsigned int xPos, unsigned int yPos) {}
};

} // namespace DXD
