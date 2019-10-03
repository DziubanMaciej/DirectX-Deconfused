#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/TextEnums.h>
#include <DXD/ExternalHeadersWrappers/d2d.h>
#include <memory>
#include <string>

namespace DXD {

class EXPORT Text : NonCopyableAndMovable {
public:
    static std::unique_ptr<Text> create(std::wstring text = L"",
                                        D2D1_COLOR_F color = D2D1::ColorF(D2D1::ColorF::Green, 1.f),
                                        std::wstring fontFamilyName = L"Consolas",
                                        DXDFontWeight fontWeight = DXDFontWeight::NORMAL,
                                        DXDFontStyle fontStyle = DXDFontStyle::NORMAL,
                                        DXDFontStretch fontStretch = DXDFontStretch::NORMAL,
                                        FLOAT fontSize = 25.f,
                                        std::wstring localeName = L"pl-PL",
                                        DXDTextHorizontalAlignment horzAlignment = DXDTextHorizontalAlignment::CENTER,
                                        DXDTextVerticalAlignment verticalAlignment = DXDTextVerticalAlignment::CENTER);
    virtual void setText(std::wstring text) = 0;
    //TODO: DXD color struct/enum   virtual void setColor(std::wstring text) = 0;
    virtual void setFontFamily(std::wstring text) = 0;
    virtual void setFontWeight(DXDFontWeight weight) = 0;
    virtual void setFontStyle(DXDFontStyle style) = 0;
    virtual void setFontStretch(DXDFontStretch stretch) = 0;
    virtual void setFontSize(FLOAT size) = 0;
    virtual void setLocale(std::wstring locale) = 0;
    virtual void setAlignment(DXDTextHorizontalAlignment ha, DXDTextVerticalAlignment va) = 0;
    virtual void setVerticalAlignment(DXDTextVerticalAlignment va) = 0;
    virtual void setHorizontalAlignment(DXDTextHorizontalAlignment ha) = 0;

    virtual ~Text() = default;

protected:
    Text() = default;
};

} // namespace DXD
