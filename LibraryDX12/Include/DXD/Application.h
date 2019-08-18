#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"

#include <memory>
#include <string>

namespace DXD {

class CallbackHandler;
class Settings;

class EXPORT Application : NonCopyableAndMovable {
public:
    virtual void setCallbackHandler(CallbackHandler *callbackHandler) = 0;
    virtual Settings &getSettings() = 0;

    virtual ~Application() = default;
    static std::unique_ptr<Application> create(bool debugLayer);

protected:
    Application() = default;
};

} // namespace DXD
