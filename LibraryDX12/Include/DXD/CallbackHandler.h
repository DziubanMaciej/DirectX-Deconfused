#pragma once

#include "DXD/Utility/Export.h"

namespace DXD {

class Window;

/// \brief Class handling all internal engine events
///
/// Application can override any availble event and implement custom handling procedure.
/// Adding event handling does not alter general engine workflow, e.g. overriding onResize
/// will not break resizing behaviour of DXD, but rather allow injecting some code to
/// execute when resize event is triggered.
class EXPORT CallbackHandler {
public:
    /// Called when the active window is resized. Also called upon minimalization with
    /// both parameters set to 0
    /// \param newWidth width of the window
    /// \param newHeight height of the window
    virtual void onResize(int newWidth, int newHeight) {}

    /// Called when user presses keyboard key
    /// \param vkCode code of pressed key. See https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes for reference
    virtual void onKeyDown(unsigned int vkCode) {}

    /// Called when user releases keyboard key
    /// \param vkCode code of released key. See https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes for reference
    virtual void onKeyUp(unsigned int vkCode) {}

    /// Called each frame before actual rendering is performed
    /// \param deltaTimeMicroseconds time elapsed since last frame
    virtual void onUpdate(unsigned int deltaTimeMicroseconds) {}

    /// Called when user scrolls mouse
    /// \param zDelta difference in wheel position, positive for scrolling up, negative for scrolling down
    virtual void onMouseWheel(int zDelta) {}

    /// Called when user presses left mouse button
    /// \param xPos screen x coordinate of the mouse during event
    /// \param yPos screen y coordinate of the mouse during event
    virtual void onLButtonUp(unsigned int xPos, unsigned int yPos) {}

    /// Called when user releases left mouse button
    /// \param xPos screen x coordinate of the mouse during event
    /// \param yPos screen y coordinate of the mouse during event
    virtual void onLButtonDown(unsigned int xPos, unsigned int yPos) {}

    /// Called when user moves mouse
    /// \param xPos new screen x coordinate of the mouse
    /// \param yPos new screen y coordinate of the mouse
    virtual void onMouseMove(unsigned int xPos, unsigned int yPos) {}
};

} // namespace DXD
