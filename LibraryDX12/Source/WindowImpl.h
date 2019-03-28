#pragma once

#include "DXD/Window.h"

class WindowImpl : Window {
protected:
    friend class Window;
    WindowImpl(const std::wstring &windowClassName, const std::wstring &windowTitle, HINSTANCE hInstanc, Bounds bounds);

public:
    ~WindowImpl() override;

    void setShowState(int nCmdShow) override;
    void show() override;
    void messageLoop() override;
    void destroy() override;

    HINSTANCE getInstance() const override;
    HWND getHandle() const override;

protected:
    // Construction/destruction helpers
    void registerClass();
    HWND createWindow(const std::wstring &windowTitle, Bounds bounds);
    void destroyWindow();
    void unregisterClass();

    // Window Procedures
    static LRESULT CALLBACK windowProcNative(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam); // called by win32
    LRESULT windowProcImpl(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);                   // actual procedure, called by windowProcNative

    // Window instance accessors (for WNDPROC usage)
    static void setWindowInstanceForHandle(HWND windowHandle, WindowImpl *windowProcImpl);
    static WindowImpl *getWindowInstanceFromHandle(HWND windowHandle);
    static WindowImpl *getWindow(HWND windowHandle, UINT message, LPARAM lParam);

    const std::wstring windowClassName;
    const HINSTANCE hInstance;
    HWND windowHandle;
};
