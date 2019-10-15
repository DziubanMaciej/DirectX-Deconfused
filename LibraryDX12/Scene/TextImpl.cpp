#include "TextImpl.h"

#include "Application/ApplicationImpl.h"
#include "Utility/ThrowIfFailed.h"

// ----------------------------------------------------------------------- Creating

namespace DXD {
std::unique_ptr<Text> DXD::Text::create() {
    return std::make_unique<TextImpl>();
}
} // namespace DXD

// ----------------------------------------------------------------------- Per frame operations

void TextImpl::update() {
    if (!dirty) {
        return;
    }

    auto &context = ApplicationImpl::getInstance().getD2DContext();
    throwIfFailed(context.getD2DDeviceContext()->CreateSolidColorBrush(color, &m_textBrush));
    throwIfFailed(context.getDWriteFactory()->CreateTextFormat(
        fontFamilyName.c_str(),
        NULL,
        static_cast<DWRITE_FONT_WEIGHT>(fontWeight),
        static_cast<DWRITE_FONT_STYLE>(fontStyle),
        static_cast<DWRITE_FONT_STRETCH>(fontStretch),
        fontSize,
        localeName.c_str(),
        &m_textFormat));
    throwIfFailed(m_textFormat->SetTextAlignment((DWRITE_TEXT_ALIGNMENT)horzAlignment));
    throwIfFailed(m_textFormat->SetParagraphAlignment((DWRITE_PARAGRAPH_ALIGNMENT)vertAlignment));
    dirty = false;
}

void TextImpl::draw(const D2D1_RECT_F &textRect) {
    ID2D1DeviceContextPtr context = ApplicationImpl::getInstance().getD2DContext().getD2DDeviceContext();
    context->DrawTextA(
        this->text.c_str(),
        static_cast<UINT>(this->text.size()),
        this->m_textFormat.Get(),
        &textRect,
        this->m_textBrush.Get());
}

// ----------------------------------------------------------------------- Setters

void TextImpl::setText(std::wstring text) {
    this->text = text;
}

void TextImpl::setColor(float r, float g, float b, float a) {
    dirty = true;
    this->color.r = r;
    this->color.g = g;
    this->color.b = b;
    this->color.a = a;
}

void TextImpl::setFontFamily(std::wstring family) {
    dirty = true;
    this->fontFamilyName = family;
}

void TextImpl::setFontWeight(DXDFontWeight weight) {
    dirty = true;
    this->fontWeight = weight;
}

void TextImpl::setFontStyle(DXDFontStyle style) {
    dirty = true;
    this->fontStyle = style;
}

void TextImpl::setFontStretch(DXDFontStretch stretch) {
    dirty = true;
    this->fontStretch = fontStretch;
}

void TextImpl::setFontSize(FLOAT size) {
    dirty = true;
    this->fontSize = size;
}

void TextImpl::setLocale(std::wstring locale) {
    dirty = true;
    this->localeName = locale;
}

void TextImpl::setAlignment(DXDTextHorizontalAlignment ha, DXDTextVerticalAlignment va) {
    dirty = true;
    this->horzAlignment = ha;
    this->vertAlignment = va;
}

void TextImpl::setVerticalAlignment(DXDTextVerticalAlignment va) {
    dirty = true;
    this->vertAlignment = va;
}

void TextImpl::setHorizontalAlignment(DXDTextHorizontalAlignment ha) {
    dirty = true;
    this->horzAlignment = ha;
}
