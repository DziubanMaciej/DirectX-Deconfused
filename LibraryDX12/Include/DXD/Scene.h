#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"
#include <memory>

namespace DXD {
class EXPORT Scene : NonCopyableAndMovable {
public:
    virtual void setBackgroundColor(float r, float g, float b) = 0;

    virtual ~Scene() = default;
    static std::unique_ptr<Scene> create();

protected:
    Scene() = default;
};
} // namespace DXD
