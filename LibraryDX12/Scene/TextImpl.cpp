#include "TextImpl.h"

#include "Application/ApplicationImpl.h"
#include "Utility/ThrowIfFailed.h"

namespace DXD {
std::unique_ptr<Text> DXD::Text::create(std::wstring text,
                                        D2D1_COLOR_F color,
                                        std::wstring fontFamilyName,
                                        DXDFontWeight fontWeight,
                                        DXDFontStyle fontStyle,
                                        DXDFontStretch fontStretch,
                                        FLOAT fontSize,
                                        std::wstring localeName,
                                        DXDTextHorizontalAlignment horzAlignment,
                                        DXDTextVerticalAlignment verticalAlignment) {
    return std::make_unique<TextImpl>(text, color, fontFamilyName, fontWeight, fontStyle, fontStretch, fontSize, localeName, horzAlignment, verticalAlignment);
}
} // namespace DXD
TextImpl::TextImpl(std::wstring text,
                   D2D1_COLOR_F color,
                   std::wstring fontFamilyName,
                   DXDFontWeight fontWeight,
                   DXDFontStyle fontStyle,
                   DXDFontStretch fontStretch,
                   FLOAT fontSize,
                   std::wstring localeName,
                   DXDTextHorizontalAlignment horzAlignment,
                   DXDTextVerticalAlignment verticalAlignment)
    : text(text), color(color), fontFamilyName(fontFamilyName), fontWeight(fontWeight), fontStyle(fontStyle), fontStretch(fontStretch),
      fontSize(fontSize), localeName(localeName), horzAlignment(horzAlignment), vertAlignment(verticalAlignment), dirty(true) {
}

void TextImpl::updateText() {
    if (!dirty)
        return;
    auto &application = ApplicationImpl::getInstance();
    throwIfFailed(application.m_d2dDeviceContext->CreateSolidColorBrush(color, &m_textBrush));
    throwIfFailed(application.m_dWriteFactory->CreateTextFormat(
        fontFamilyName.c_str(),
        NULL,
        (DWRITE_FONT_WEIGHT)fontWeight,
        (DWRITE_FONT_STYLE)fontStyle,
        (DWRITE_FONT_STRETCH)fontStretch,
        fontSize,
        localeName.c_str(),
        &m_textFormat));
    throwIfFailed(m_textFormat->SetTextAlignment((DWRITE_TEXT_ALIGNMENT)horzAlignment));
    throwIfFailed(m_textFormat->SetParagraphAlignment((DWRITE_PARAGRAPH_ALIGNMENT)vertAlignment));
    dirty = false;
}