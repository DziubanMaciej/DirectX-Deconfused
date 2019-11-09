#pragma once
#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

namespace DXD {
class EXPORT Settings : NonCopyableAndMovable {
public:
    virtual ~Settings() = default;

    virtual void setVerticalSyncEnabled(bool value) = 0;
    virtual void setSsaoEnabled(bool value) = 0;
    virtual void setSsrEnabled(bool value) = 0;

    virtual bool getVerticalSyncEnabled() const = 0;
    virtual bool getSsaoEnabled() const = 0;
    virtual bool getSsrEnabled() const = 0;
};
} // namespace DXD
