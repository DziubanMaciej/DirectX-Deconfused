#include "WindowImpl.h"

#include <algorithm>
#include <cassert>

// -------------------------------------------------------------------------------- Creating

std::unique_ptr<Window> Window::create(const std::wstring &windowClassName, const std::wstring &windowTitle,
                                       HINSTANCE hInstance, Bounds bounds) {
    return std::unique_ptr<Window>{new WindowImpl(windowClassName, windowTitle, hInstance, bounds)};
}

std::unique_ptr<Window> Window::create(const std::wstring &windowClassName, const std::wstring &windowTitle,
                                       HINSTANCE hInstance, int width, int height) {
    const auto screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    const auto screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    RECT windowRect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    const auto windowWidth = static_cast<int>(windowRect.right - windowRect.left);
    const auto windowLeft = std::max<int>(0, (screenWidth - windowWidth) / 2);
    const auto windowHeight = static_cast<int>(windowRect.bottom - windowRect.top);
    const auto windowTop = std::max<int>(0, (screenHeight - windowHeight) / 2);

    Window::Bounds bounds{windowLeft, windowTop, windowWidth, windowHeight};
    return Window::create(windowClassName, windowTitle, hInstance, bounds);
}

WindowImpl::WindowImpl(const std::wstring &windowClassName, const std::wstring &windowTitle, HINSTANCE hInstance, Bounds bounds)
    : windowClassName(windowClassName), hInstance(hInstance) {
    registerClass();
    this->windowHandle = createWindow(windowTitle, bounds);
}

void WindowImpl::registerClass() {
    // TODO assert if there is a class with given name
    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = windowProcNative;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = ::LoadIcon(hInstance, NULL);
    windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = windowClassName.c_str();
    windowClass.hIconSm = ::LoadIcon(hInstance, NULL);
    ATOM atom = ::RegisterClassExW(&windowClass);
    assert(atom > 0);
}

HWND WindowImpl::createWindow(const std::wstring &windowTitle, Bounds bounds) {
    HWND windowHandle = ::CreateWindowExW(
        NULL, windowClassName.c_str(), windowTitle.c_str(),
        WS_OVERLAPPEDWINDOW, bounds.x, bounds.y, bounds.width, bounds.height,
        NULL, NULL, hInstance, this);
    assert(windowHandle != NULL);
    return windowHandle;
}

// -------------------------------------------------------------------------------- Destroying

WindowImpl::~WindowImpl() {
    destroyWindow();
    unregisterClass();
}

void WindowImpl::destroyWindow() {
    if (getWindowInstanceFromHandle(windowHandle) == nullptr) {
        return;
    }

    setWindowInstanceForHandle(windowHandle, nullptr);
    const auto destroyResult = DestroyWindow(windowHandle);
    assert(destroyResult != 0);
}

void WindowImpl::unregisterClass() {
    const auto unregisterResult = UnregisterClassW(windowClassName.c_str(), hInstance);
    assert(unregisterResult != 0);
}

// -------------------------------------------------------------------------------- Window procedure

LRESULT WindowImpl::windowProcNative(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
    // Get window instance
    WindowImpl *window = getWindow(windowHandle, message, lParam);

    // Process simple messages
    switch (message) {
    case WM_DESTROY:
        setWindowInstanceForHandle(windowHandle, nullptr);
        ::PostQuitMessage(0);
        return 0;
    case WM_SYSCHAR: // empty handling of Alt+Key combinations
        OutputDebugString((std::string{"WM_KEYDOWN "} + std::to_string(wParam) + " " + std::to_string(lParam) + "\n").c_str());
        return 0;
    }

    // If there is no window, it's been destroyed, ignore
    if (window == nullptr) {
        return ::DefWindowProcW(windowHandle, message, wParam, lParam);
    }

    // Call windowProcImpl
    return window->windowProcImpl(windowHandle, message, wParam, lParam);
}

LRESULT WindowImpl::windowProcImpl(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_PAINT:
        break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            destroy();
        }
        break;
    case WM_SIZE:
        break;
    default:
        return ::DefWindowProcW(windowHandle, message, wParam, lParam);
    }
    return 0;
}

// -------------------------------------------------------------------------------- Window instance accessors

void WindowImpl::setWindowInstanceForHandle(HWND windowHandle, WindowImpl *windowProcWithDataForHandle) {
    SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowProcWithDataForHandle));
}

WindowImpl *WindowImpl::getWindowInstanceFromHandle(HWND windowHandle) {
    return reinterpret_cast<WindowImpl *>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));
}

WindowImpl *WindowImpl::getWindow(HWND windowHandle, UINT message, LPARAM lParam) {
    WindowImpl *result;
    if (message == WM_CREATE) {
        result = reinterpret_cast<WindowImpl *>(reinterpret_cast<CREATESTRUCT *>(lParam)->lpCreateParams);
        setWindowInstanceForHandle(windowHandle, result);
    } else {
        result = getWindowInstanceFromHandle(windowHandle);
    }
    return result;
}

// --------------------------------------------------------------------------------

void WindowImpl::setShowState(int nCmdShow) {
    ::ShowWindow(getHandle(), nCmdShow);
}

void WindowImpl::show() {
    setShowState(SW_SHOW);
}

void WindowImpl::messageLoop() {
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }
}

void WindowImpl::destroy() {
    ::DestroyWindow(windowHandle);
}

HINSTANCE WindowImpl::getInstance() const {
    return hInstance;
}

HWND WindowImpl::getHandle() const {
    return windowHandle;
}
