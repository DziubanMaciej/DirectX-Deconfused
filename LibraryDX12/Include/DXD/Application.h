#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <memory>
#include <string>

namespace DXD {

class CallbackHandler;
class Settings;

/// \brief Encapsulates global state of the graphics application
///
/// Basic class for internal engine usage containing data, constants, information and subsystems common
/// to the entire engine. There should be precisely one instance of the Application class during correct
/// application execution. It should be created before any other DXD object and destroyed after all DXD
/// objects have been freed as well
class EXPORT Application : NonCopyableAndMovable {
public:
    /// Enumeration describing, what engine should do when the window is minimized
    enum class MinimizeBehavior {
        /// Engine will try to free as many memory allocations as possible
        Deallocate,

        /// Engine will not react to minimize event
        Keep,
    };

    /// Set active CallbackHandler instance which will be handling all internal engine events.
    /// \param callbackHandler object to set as the active instance, can be null
    virtual void setCallbackHandler(CallbackHandler *callbackHandler) = 0;

    /// Acquire instance to application's settings object
    /// \return Settings class instance
    virtual Settings &getSettings() = 0;

    /// Factory function used to create Application instance. This function should be called only once during
    /// whole execution and should be the first DXD function called.
    /// \param debugLayer enable diagnostic DirectX debug layer, should be set to false during normal development
    /// \param debugShaders enable debug info in shaders, should be set to false during normal development
    /// \param minimizeBehavior description of engine reaction to minimize event
    /// \return Application instance
    static std::unique_ptr<Application> create(bool debugLayer, bool debugShaders, MinimizeBehavior minimizeBehavior);
    virtual ~Application() = default;

protected:
    Application() = default;
};

} // namespace DXD
