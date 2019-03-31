#pragma once

#include "DXD/Export.h"
#include <memory>
#include <string>

namespace DXD {
class Window;

class EXPORT CallbackHandler {
public:
    virtual void onResize(int newWidth, int newHeight) {}
    virtual void onWindowShow(Window &window) {}
    virtual void onKeyDown(unsigned int vkCode) {}
    virtual void onKeyUp(unsigned int vkCode) {}
    virtual void onUpdate() {}
};
} // namespace DXD
