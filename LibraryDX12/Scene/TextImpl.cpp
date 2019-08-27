#include "TextImpl.h"

#include "Application/ApplicationImpl.h"
#include "Utility/ThrowIfFailed.h"

TextImpl::TextImpl() {
    createText(D2D1::ColorF(D2D1::ColorF::Green, 1.f), L"Comic Sans MS", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
               DWRITE_FONT_STRETCH_NORMAL, 25., L"pl-PL", DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
}

TextImpl::TextImpl(std::wstring text, D2D1_COLOR_F color, std::wstring fontFamilyName, IDWriteFontCollection *fontCollection,
                   DXDFontWeight fontWeight, DXDFontStyle fontStyle, DXDFontStretch fontStretch, FLOAT fontSize,
                   std::wstring localeName, DXDTextHorizontalAlignment textAlignment, DXDTextVerticalAlignment paragraphAlignment) : text(text) {
    createText(color, fontFamilyName, fontCollection, (DWRITE_FONT_WEIGHT)fontWeight, (DWRITE_FONT_STYLE)fontStyle, (DWRITE_FONT_STRETCH)fontStretch, fontSize, localeName, (DWRITE_TEXT_ALIGNMENT)textAlignment, (DWRITE_PARAGRAPH_ALIGNMENT)paragraphAlignment);
}

void TextImpl::createText(const D2D1_COLOR_F color, std::wstring fontFamilyName, IDWriteFontCollection *fontCollection,
                          DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontStretch, FLOAT fontSize,
                          std::wstring localeName, DWRITE_TEXT_ALIGNMENT textAlignment, DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment) {
    auto &application = ApplicationImpl::getInstance();
    throwIfFailed(application.m_d2dDeviceContext->CreateSolidColorBrush(color, &m_textBrush));
    throwIfFailed(application.m_dWriteFactory->CreateTextFormat(
        fontFamilyName.c_str(),
        fontCollection,
        fontWeight,
        fontStyle,
        fontStretch,
        fontSize,
        localeName.c_str(),
        &m_textFormat));
    throwIfFailed(m_textFormat->SetTextAlignment(textAlignment));
    throwIfFailed(m_textFormat->SetParagraphAlignment(paragraphAlignment));
}