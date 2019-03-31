#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"
#include "DXD/ExternalHeadersWrappers/windows.h"
#include <memory>
#include <string>

namespace DXD {
class Application;
class Scene;

class EXPORT Window : NonCopyableAndMovable {
public:
    struct Bounds {
        int x, y, width, height;
    };

    virtual void setScene(Scene &scene) = 0;
    virtual void setShowState(int nCmdShow) = 0;
    virtual void show() = 0;
    virtual void messageLoop() = 0;
    virtual void destroy() = 0;
    virtual HINSTANCE getInstance() const = 0;
    virtual HWND getHandle() const = 0;

    virtual ~Window() = default;
    static std::unique_ptr<Window> create(Application &application, const std::wstring &windowClassName, const std::wstring &windowTitle, HINSTANCE hInstance, Bounds bounds);
    static std::unique_ptr<Window> create(Application &application, const std::wstring &windowClassName, const std::wstring &windowTitle, HINSTANCE hInstance, int width, int height);

protected:
    Window() = default;
};
} // namespace DXD
