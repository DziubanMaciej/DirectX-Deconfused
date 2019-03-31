#pragma once

#include "DXD/CallbackHandler.h"
#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"
#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <memory>
#include <string>

namespace DXD {
class EXPORT Application : NonCopyableAndMovable {
public:
    virtual void setCallbackHandler(CallbackHandler *callbackHandler) = 0;

    virtual ~Application() = default;
    static std::unique_ptr<Application> create(bool debugLayer);

protected:
    Application() = default;
};
} // namespace DXD
