#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"

#include <memory>
#include <string>
#include "DXD/ExternalHeadersWrappers/TextEnums.h"

namespace DXD {

class EXPORT Text : NonCopyableAndMovable {
public:
    static std::unique_ptr<Text> create();
    virtual void setText(std::wstring text) = 0;
    virtual ~Text() = default;

protected:
    Text() = default;
};

} // namespace DXD
