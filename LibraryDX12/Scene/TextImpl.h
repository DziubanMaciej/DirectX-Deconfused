#pragma once

#include "DXD/Text.h"

#include <DXD/ExternalHeadersWrappers/d2d.h>
#include <ExternalHeaders/Wrappers/d3dx12.h>

class TextImpl : public DXD::Text {
protected:
    friend class DXD::Text;

public:
    // Per frame operations
    void draw(const D2D1_RECT_F &textRect);
    void update();

    // Setters
    void setText(std::wstring text) override;
    void setColor(float r, float g, float b, float a) override;
    void setFontFamily(std::wstring family) override;
    void setFontWeight(DXDFontWeight weight) override;
    void setFontStyle(DXDFontStyle style) override;
    void setFontStretch(DXDFontStretch stretch) override;
    void setFontSize(FLOAT size) override;
    void setLocale(std::wstring locale) override;
    void setAlignment(DXDTextHorizontalAlignment ha, DXDTextVerticalAlignment va) override;
    void setVerticalAlignment(DXDTextVerticalAlignment va) override;
    void setHorizontalAlignment(DXDTextHorizontalAlignment ha) override;

protected:
    // Textual content
    std::wstring text = {};

    // Api-visible settings
    D2D1_COLOR_F color = {1, 1, 1, 1};
    std::wstring fontFamilyName = L"Consolas";
    DXDFontWeight fontWeight = DXDFontWeight::NORMAL;
    DXDFontStyle fontStyle = DXDFontStyle::NORMAL;
    DXDFontStretch fontStretch = DXDFontStretch::NORMAL;
    FLOAT fontSize = 1.f;
    std::wstring localeName = L"pl-PL";
    DXDTextHorizontalAlignment horzAlignment = DXDTextHorizontalAlignment::CENTER;
    DXDTextVerticalAlignment vertAlignment = DXDTextVerticalAlignment::CENTER;

    // Internal state
    IDWriteTextFormatPtr m_textFormat = {};
    ID2D1SolidColorBrushPtr m_textBrush = {};
    bool dirty = false;
};
