#include "WindowImpl.h"

#include "Application/ApplicationImpl.h"
#include "Scene/SceneImpl.h"
#include "Window/WindowClassFactory.h"

#include "DXD/CallbackHandler.h"
#include "DXD/Logger.h"

#include <algorithm>
#include <cassert>
#include <windowsx.h>

// -------------------------------------------------------------------------------- Creating

namespace DXD {
std::unique_ptr<Window> Window::create(const std::wstring &windowTitle, HINSTANCE hInstance, unsigned int backBuffersCount, Bounds bounds) {
    return std::unique_ptr<Window>{new WindowImpl(windowTitle, hInstance, backBuffersCount, bounds)};
}

std::unique_ptr<Window> Window::create(const std::wstring &windowTitle, HINSTANCE hInstance, unsigned int backBuffersCount, int width, int height) {
    const auto screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    const auto screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    RECT windowRect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    const auto windowWidth = static_cast<int>(windowRect.right - windowRect.left);
    const auto windowLeft = std::max<int>(0, (screenWidth - windowWidth) / 2);
    const auto windowHeight = static_cast<int>(windowRect.bottom - windowRect.top);
    const auto windowTop = std::max<int>(0, (screenHeight - windowHeight) / 2);

    Window::Bounds bounds{windowLeft, windowTop, windowWidth, windowHeight};
    return Window::create(windowTitle, hInstance, backBuffersCount, bounds);
}
} // namespace DXD

WindowImpl::WindowImpl(const std::wstring &windowTitle, HINSTANCE hInstance, unsigned int backBuffersCount, Bounds bounds)
    : application(ApplicationImpl::getInstance()),
      windowClassName(WindowClassFactory::createWindowClassName()),
      hInstance(hInstance),
      windowHandle(registerClassAndCreateWindow(windowTitle, bounds)),
      lastFrameTime(Clock::now()),
      swapChain(windowHandle, application.getDirectCommandQueue(), bounds.width, bounds.height, backBuffersCount),
      renderData(bounds.width, bounds.height, backBuffersCount) {}

HWND WindowImpl::registerClassAndCreateWindow(const std::wstring &windowTitle, Bounds bounds) {
    registerClass();
    return createWindow(windowTitle, bounds);
}

void WindowImpl::registerClass() {
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
        DXD::log("WM_KEYDOWN %d %d\n", wParam, lParam);
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
        handlePaint();
        break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        handleKeyDown(static_cast<unsigned int>(wParam));
        break;
    case WM_SYSKEYUP:
    case WM_KEYUP:
        handleKeyUp(static_cast<unsigned int>(wParam));
        break;
    case WM_SIZE:
        handleResize();
        break;
    case WM_MOUSEWHEEL:
        handleMouseWheel(static_cast<int>(GET_WHEEL_DELTA_WPARAM(wParam)));
        break;
    case WM_LBUTTONUP:
        handleLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;
    case WM_LBUTTONDOWN:
        handleLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;
    case WM_MOUSEMOVE:
        handleMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;
    default:
        return ::DefWindowProcW(windowHandle, message, wParam, lParam);
    }
    return 0;
}

// -------------------------------------------------------------------------------- Window events handlers

void WindowImpl::handlePaint() {
    const auto currentTime = Clock::now();
    const auto deltaTimeMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - this->lastFrameTime);
    this->lastFrameTime = currentTime;

    auto handler = application.getCallbackHandler();
    if (handler != nullptr) {
        handler->onUpdate(static_cast<unsigned int>(deltaTimeMicroseconds.count()));
    }

    if (scene != nullptr && clientWidth > 0) {
        scene->render(swapChain, renderData);
    }
}

void WindowImpl::handleKeyDown(unsigned int vkCode) {
    auto handler = application.getCallbackHandler();
    if (handler != nullptr) {
        handler->onKeyDown(vkCode);
    }
}

void WindowImpl::handleKeyUp(unsigned int vkCode) {
    auto handler = application.getCallbackHandler();
    if (handler != nullptr) {
        handler->onKeyUp(vkCode);
    }
}

void WindowImpl::handleResize() {
    // Get old and new size
    RECT clientRect = {};
    ::GetClientRect(windowHandle, &clientRect);
    const int newWidth = clientRect.right - clientRect.left;
    const int newHeight = clientRect.bottom - clientRect.top;

    const bool sizeChanged = this->clientWidth != newWidth || this->clientHeight != newHeight;
    const bool isMaximizedOrMinimized = ((this->clientWidth == 0) ^ (newWidth == 0));
    const bool shouldReactToMinimize = application.getMinimizeBehavior() == DXD::Application::MinimizeBehavior::Deallocate;
    if (sizeChanged && (shouldReactToMinimize || !isMaximizedOrMinimized)) {
        // Finish all work
        application.flushAllQueues();
        application.flushAllResources();

        // Resize allocations
        swapChain.resize(newWidth, newHeight);
        renderData.resize(newWidth, newHeight);
    }

    auto handler = application.getCallbackHandler();
    if (handler != nullptr) {
        handler->onResize(newWidth, newHeight);
    }

    this->clientWidth = newWidth;
    this->clientHeight = newHeight;
}

void WindowImpl::handleMouseWheel(int zDelta) {
    auto handler = application.getCallbackHandler();
    if (handler != nullptr) {
        handler->onMouseWheel(zDelta);
    }
}

void WindowImpl::handleLButtonUp(unsigned int xPos, unsigned int yPos) {
    auto handler = application.getCallbackHandler();
    if (handler != nullptr) {
        handler->onLButtonUp(xPos, yPos);
    }
}

void WindowImpl::handleLButtonDown(unsigned int xPos, unsigned int yPos) {
    auto handler = application.getCallbackHandler();
    if (handler != nullptr) {
        handler->onLButtonDown(xPos, yPos);
    }
}

void WindowImpl::handleMouseMove(unsigned int xPos, unsigned int yPos) {
    auto handler = application.getCallbackHandler();
    if (handler != nullptr) {
        handler->onMouseMove(xPos, yPos);
    }
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

void WindowImpl::setFullscreen(bool fullscreen) {
    if (fullscreenData.enabled == fullscreen) {
        return;
    }

    if (fullscreen) {
        // Store current bounds and style
        RECT rect;
        ::GetWindowRect(windowHandle, &rect);
        fullscreenData.savedBounds.x = rect.left;
        fullscreenData.savedBounds.y = rect.top;
        fullscreenData.savedBounds.width = rect.right - rect.left;
        fullscreenData.savedBounds.height = rect.bottom - rect.top;
        fullscreenData.savedStyle = ::GetWindowLong(windowHandle, GWL_STYLE);

        // Set the window style to a borderless window so the client area fills the entire screen.
        UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        ::SetWindowLongW(windowHandle, GWL_STYLE, windowStyle);

        // Query the name of monitor that is nearest to the window. Required to set dimensions for multi-monitor setup
        HMONITOR hMonitor = ::MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
        MONITORINFOEX monitorInfo = {};
        monitorInfo.cbSize = sizeof(MONITORINFOEX);
        ::GetMonitorInfo(hMonitor, &monitorInfo);

        // Set dimensions to size of the monitor and maximize
        ::SetWindowPos(windowHandle, HWND_TOPMOST,
                       monitorInfo.rcMonitor.left,
                       monitorInfo.rcMonitor.top,
                       monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                       monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                       SWP_FRAMECHANGED | SWP_NOACTIVATE);
        ::ShowWindow(windowHandle, SW_MAXIMIZE);
    } else {
        // Restore all the window decorators.
        ::SetWindowLong(windowHandle, GWL_STYLE, fullscreenData.savedStyle);
        ::SetWindowPos(windowHandle, HWND_NOTOPMOST,
                       fullscreenData.savedBounds.x, fullscreenData.savedBounds.y,
                       fullscreenData.savedBounds.width, fullscreenData.savedBounds.height,
                       SWP_FRAMECHANGED | SWP_NOACTIVATE);
        ::ShowWindow(windowHandle, SW_NORMAL);
    }
    fullscreenData.enabled = fullscreen;
}

void WindowImpl::setScene(DXD::Scene &scene) {
    this->scene = reinterpret_cast<SceneImpl *>(&scene);
}

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
    // Pushes close event to the message queue. DestroyWindow works synchronously and fails if called during onUpdate()
    ::PostMessage(windowHandle, WM_CLOSE, 0, 0);
}

HINSTANCE WindowImpl::getInstance() const {
    return hInstance;
}

HWND WindowImpl::getHandle() const {
    return windowHandle;
}
