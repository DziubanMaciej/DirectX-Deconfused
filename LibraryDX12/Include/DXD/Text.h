#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/TextEnums.h>
#include <DXD/ExternalHeadersWrappers/d2d.h>
#include <memory>
#include <string>

namespace DXD {

/// \brief Text displayed on screen
class EXPORT Text : NonCopyableAndMovable {
public:
    virtual void setText(std::wstring text) = 0;
    virtual void setColor(float r, float g, float b, float a) = 0;
    virtual void setFontFamily(std::wstring text) = 0;
    virtual void setFontWeight(DXDFontWeight weight) = 0;
    virtual void setFontStyle(DXDFontStyle style) = 0;
    virtual void setFontStretch(DXDFontStretch stretch) = 0;
    virtual void setFontSize(FLOAT size) = 0;
    virtual void setLocale(std::wstring locale) = 0;
    virtual void setAlignment(DXDTextHorizontalAlignment ha, DXDTextVerticalAlignment va) = 0;
    virtual void setVerticalAlignment(DXDTextVerticalAlignment va) = 0;
    virtual void setHorizontalAlignment(DXDTextHorizontalAlignment ha) = 0;

    static std::unique_ptr<Text> create();
    virtual ~Text() = default;

protected:
    Text() = default;
};

} // namespace DXD
