#pragma once

#include "DXD/CallbackHandler.h"
#include "DXD/Export.h"
#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include "DXD/NonCopyableAndMovable.h"

#include <memory>
#include <string>

class EXPORT Application : NonCopyableAndMovable {
public:
    virtual void setCallbackHandler(CallbackHandler *callbackHandler) = 0;

    virtual ~Application() = default;
    static std::unique_ptr<Application> create(bool debugLayer);

protected:
    Application() = default;
};
