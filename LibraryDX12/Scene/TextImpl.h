#pragma once

#include "DXD/Text.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <d2d1_3.h>
#include <d3d11.h>
#include <d3d11on12.h>
#include <dwrite.h>

class TextImpl : public DXD::Text {
protected:
    friend class DXD::Text;

public:
    TextImpl();
    TextImpl(std::wstring text, D2D1_COLOR_F color, std::wstring fontFamilyName, IDWriteFontCollection *fontCollection,
             DXDFontWeight fontWeight, DXDFontStyle fontStyle, DXDFontStretch fontStretch, FLOAT fontSize,
             std::wstring localeName, DXDTextHorizontalAlignment textAlignment, DXDTextVerticalAlignment paragraphAlignment);
    ~TextImpl() = default;

    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;

    void createText(const D2D1_COLOR_F color, std::wstring fontFamilyName, IDWriteFontCollection *fontCollection,
                    DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontStretch, FLOAT fontSize,
                    std::wstring localeName, DWRITE_TEXT_ALIGNMENT textAlignment, DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment);

    std::wstring getText() { return text; }
    void setText(std::wstring text) { this->text = text; }

protected:
    std::wstring text;
};
