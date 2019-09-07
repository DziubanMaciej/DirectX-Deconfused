#pragma once

#include "DXD/Text.h"

#include "DXD/ExternalHeadersWrappers/d2d.h"
#include "DXD/ExternalHeadersWrappers/d3dx12.h"

class TextImpl : public DXD::Text {
protected:
    friend class DXD::Text;

public:
    TextImpl(std::wstring text, D2D1_COLOR_F color, std::wstring fontFamilyName,
             DXDFontWeight fontWeight, DXDFontStyle fontStyle, DXDFontStretch fontStretch, FLOAT fontSize,
             std::wstring localeName, DXDTextHorizontalAlignment textAlignment, DXDTextVerticalAlignment paragraphAlignment);
    ~TextImpl() = default;

    IDWriteTextFormatPtr m_textFormat;
    ID2D1SolidColorBrushPtr m_textBrush;

    void createText(const D2D1_COLOR_F color, std::wstring fontFamilyName, IDWriteFontCollection *fontCollection,
                    DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontStretch, FLOAT fontSize,
                    std::wstring localeName, DWRITE_TEXT_ALIGNMENT textAlignment, DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment);

    std::wstring getText() { return text; }

    // TODO: move overrides to cpp file
    void setText(std::wstring text) override {
        //TODO: dirty = true;
        this->text = text;
    }
    //    void setColor(std::wstring text) override{};
    void setFontFamily(std::wstring family) override {
        dirty = true;
        this->fontFamilyName = family;
    };
    void setFontWeight(DXDFontWeight weight) override {
        dirty = true;
        this->fontWeight = weight;
    };
    void setFontStyle(DXDFontStyle style) override {
        dirty = true;
        this->fontStyle = style;
    }
    void setFontStretch(DXDFontStretch stretch) override {
        dirty = true;
        this->fontStretch = fontStretch;
    }
    void setFontSize(FLOAT size) override {
        dirty = true;
        this->fontSize = size;
    }
    void setLocale(std::wstring locale) override {
        dirty = true;
        this->localeName = locale;
    }
    void setAlignment(DXDTextHorizontalAlignment ha, DXDTextVerticalAlignment va) override {
        dirty = true;
        this->horzAlignment = ha;
        this->vertAlignment = va;
    }
    void setVerticalAlignment(DXDTextVerticalAlignment va) override {
        dirty = true;
        this->vertAlignment = va;
    }
    void setHorizontalAlignment(DXDTextHorizontalAlignment ha) override {
        dirty = true;
        this->horzAlignment = ha;
    }
    void updateText();

protected:
    std::wstring text;
    D2D1_COLOR_F color;
    std::wstring fontFamilyName;
    DXDFontWeight fontWeight;
    DXDFontStyle fontStyle;
    DXDFontStretch fontStretch;
    FLOAT fontSize;
    std::wstring localeName;
    DXDTextHorizontalAlignment horzAlignment;
    DXDTextVerticalAlignment vertAlignment;
    bool dirty;
};
