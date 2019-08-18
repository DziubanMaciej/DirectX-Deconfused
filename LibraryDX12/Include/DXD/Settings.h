#pragma once
#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"

namespace DXD {
class EXPORT Settings : NonCopyableAndMovable {
public:
    virtual ~Settings() = default;
    virtual void setVerticalSyncEnabled(bool value) = 0;
    virtual bool getVerticalSyncEnabled() = 0;
};
} // namespace DXD
