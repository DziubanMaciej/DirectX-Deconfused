#pragma once

#include "Resource/ConstantBuffer.h"
#include "Resource/ConstantBuffers.h"

#include "DXD/PostProcess.h"

// TODO std::variant to hold data as union?
class PostProcessImpl : public DXD::PostProcess {
protected:
    friend class DXD::PostProcess;
    PostProcessImpl() = default;

public:
    enum class Type {
        UNDEFINED,
        BLACK_BARS,
        CONVOLUTION,
        LINEAR_COLOR_CORRECTION
    };

    void setEnabled(bool enabled) override;

    void setBlackBars(float leftMarginPercent, float rightMarginPercent, float topMarginPercent, float bottomMarginPercent) override;

    void setConvolution(float divider, XMFLOAT3X3 matrix) override;
    void setConvolution(float divider,
                        float a00, float a01, float a02,
                        float a10, float a11, float a12,
                        float a20, float a21, float a22) override;
    void setConvolutionSharpen() override;
    void setConvolutionGaussianBlur() override;
    void setConvolutionEdgeDetection() override;

    void setLinearColorCorrection(XMFLOAT3X3 matrix) override;
    void setLinearColorCorrection(float a00, float a01, float a02,
                                  float a10, float a11, float a12,
                                  float a20, float a21, float a22) override;
    void setLinearColorCorrectionSepia() override;

    auto isEnabled() const { return enabled; }
    auto getType() const { return type; }
    auto &getData() { return data; }

private:
    bool enabled = false;
    Type type = Type::UNDEFINED;

    union Data {
        PostProcessBlackBarsCB blackBars;
        PostProcessConvolutionCB convolution;
        PostProcessLinearColorCorrectionCB linearColorCorrection;
    };
    Data data = {};
};
