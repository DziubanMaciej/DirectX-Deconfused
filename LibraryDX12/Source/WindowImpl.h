#pragma once

#include "DXD/Window.h"
#include "Source/SwapChain.h"

class ApplicationImpl;

class WindowImpl : public DXD::Window {
protected:
    friend class DXD::Window;
    WindowImpl(DXD::Application &application, const std::wstring &windowClassName, const std::wstring &windowTitle, HINSTANCE hInstanc, Bounds bounds);

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
    HWND registerClassAndCreateWindow(const std::wstring &windowTitle, Bounds bounds);
    void registerClass();
    HWND createWindow(const std::wstring &windowTitle, Bounds bounds);
    void destroyWindow();
    void unregisterClass();

    // Window Procedures
    static LRESULT CALLBACK windowProcNative(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam); // called by win32
    LRESULT windowProcImpl(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);                   // actual procedure, called by windowProcNative

    // Window events handlers
    void handlePaint();
    void handleKeyDown(unsigned int vkCode);
    void handleKeyUp(unsigned int vkCode);
    void handleResize();

    // Window instance accessors (for WNDPROC usage)
    static void setWindowInstanceForHandle(HWND windowHandle, WindowImpl *windowProcImpl);
    static WindowImpl *getWindowInstanceFromHandle(HWND windowHandle);
    static WindowImpl *getWindow(HWND windowHandle, UINT message, LPARAM lParam);

    ApplicationImpl &application;
    const std::wstring windowClassName;
    const HINSTANCE hInstance;
    const HWND windowHandle;
    SwapChain swapChain;

    static constexpr uint32_t swapChainBufferCount = 3u; // TODO make this configurable from api?
};
