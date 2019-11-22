#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/windows.h>
#include <memory>
#include <string>

namespace DXD {

class Application;
class Scene;

/// \brief Application window
class EXPORT Window : NonCopyableAndMovable {
public:
    struct Bounds {
        int x, y, width, height;
    };

    /// Set fullscreen mode
    virtual void setFullscreen(bool fullscreen) = 0;

    /// Set active scene to render
    virtual void setScene(Scene &scene) = 0;

    /// @{
    virtual void setShowState(int nCmdShow) = 0;
    virtual void show() = 0;
    /// @}

    /// Main application loop which ends only after the window is destroyed. Perform all the
    /// necessary application setup, especially setting your CallbackHandler to the Application
    /// before calling this method
    virtual void messageLoop() = 0;

    /// Force close the window and end the messageLoop if running
    virtual void destroy() = 0;

    /// @{
    virtual HINSTANCE getInstance() const = 0;
    virtual HWND getHandle() const = 0;
    /// @}

    /// \name Factory methods for creating a Window
    /// @{

    /// Creates window with given position and size at the center of screen
    /// \param windowTitle title displayed at the top of the window
    /// \param hInstance handle to the win32 application
    /// \param bounds position and size of created window
    static std::unique_ptr<Window> create(const std::wstring &windowTitle, HINSTANCE hInstance, Bounds bounds);

    /// Creates window with given size at the center of screen
    /// \param windowTitle title displayed at the top of the window
    /// \hInstance handle to the win32 application
    /// \param width window width
    /// \param height window height
    static std::unique_ptr<Window> create(const std::wstring &windowTitle, HINSTANCE hInstance, int width, int height);
    /// @}
    virtual ~Window() = default;

protected:
    Window() = default;
};

} // namespace DXD
