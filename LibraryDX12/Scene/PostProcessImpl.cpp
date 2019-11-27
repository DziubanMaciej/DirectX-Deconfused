#include "PostProcessImpl.h"

namespace DXD {
std::unique_ptr<PostProcess> PostProcess::create() {
    return std::unique_ptr<PostProcess>{new PostProcessImpl()};
}
} // namespace DXD

void PostProcessImpl::setEnabled(bool enabled) {
    if (this->enabled == enabled) {
        return;
    }

    assert(!(enabled && type == Type::UNDEFINED));
    this->enabled = enabled;
}

void PostProcessImpl::setBlackBars(float leftMarginPercent, float rightMarginPercent, float topMarginPercent, float bottomMarginPercent) {
    this->type = Type::BLACK_BARS;
    this->enabled = true;

    auto &data = this->data.blackBars;
    data.screenHeight = -1;
    data.screenWidth = -1;
    data.leftMarginPercent = leftMarginPercent;
    data.rightMarginPercent = rightMarginPercent;
    data.topMarginPercent = topMarginPercent;
    data.bottomMarginPercent = bottomMarginPercent;
}

void PostProcessImpl::setConvolution(float divider, XMFLOAT3X3 matrix) {
    setConvolution(divider,
                   matrix._11, matrix._12, matrix._13,
                   matrix._21, matrix._22, matrix._23,
                   matrix._31, matrix._32, matrix._33);
}

void PostProcessImpl::setConvolution(float divider,
                                     float a00, float a01, float a02,
                                     float a10, float a11, float a12,
                                     float a20, float a21, float a22) {
    this->type = Type::CONVOLUTION;
    this->enabled = true;

    auto &data = this->data.convolution;
    data.screenHeight = -1;
    data.screenWidth = -1;
    data.divider = divider;
    data.kernel = XMFLOAT4X3(a00, a01, a02, 0.f,
                             a10, a11, a12, 0.f,
                             a20, a21, a22, 0.f);
}

void PostProcessImpl::setConvolutionSharpen() {
    setConvolution(1,
                   0, -1, 0,
                   -1, 5, -1,
                   0, -1, 0);
}

void PostProcessImpl::setConvolutionGaussianBlur() {
    setConvolution(16,
                   1, 2, 1,
                   2, 4, 2,
                   1, 2, 1);
}

void PostProcessImpl::setConvolutionEdgeDetection() {
    setConvolution(1,
                   0, 1, 0,
                   1, -4, 1,
                   0, 1, 0);
}

void PostProcessImpl::setLinearColorCorrection(float a00, float a01, float a02,
                                               float a10, float a11, float a12,
                                               float a20, float a21, float a22) {
    this->type = Type::LINEAR_COLOR_CORRECTION;
    this->enabled = true;

    auto &data = this->data.linearColorCorrection;
    data.screenWidth = -1;
    data.screenHeight = -1;
    data.colorMatrix = XMFLOAT4X3(a00, a01, a02, 0.f,
                                  a10, a11, a12, 0.f,
                                  a20, a21, a22, 0.f);
}

void PostProcessImpl::setLinearColorCorrection(XMFLOAT3X3 matrix) {
    setLinearColorCorrection(matrix._11, matrix._12, matrix._13,
                             matrix._21, matrix._22, matrix._23,
                             matrix._31, matrix._32, matrix._33);
}

void PostProcessImpl::setLinearColorCorrectionSepia() {
    setLinearColorCorrection(
        0.393f, 0.769f, 0.189f,
        0.349f, 0.686f, 0.168f,
        0.272f, 0.534f, 0.131f);
}

void PostProcessImpl::setGaussianBlur(UINT passCount, UINT samplingRange) {
    this->type = Type::GAUSSIAN_BLUR;
    this->enabled = passCount > 0u;

    constexpr static UINT maxPassCount = 10u;
    constexpr static UINT maxSamplingRange = 5u;

    auto &data = this->data.gaussianBlur;
    data.cb.screenHeight = -1;
    data.cb.screenWidth = -1;
    data.passCount = std::min(passCount, maxPassCount);
}

void PostProcessImpl::setFxaa() {
    this->type = Type::FXAA;
    this->enabled = true;

    auto &data = this->data.fxaa;
    data.screenHeight = -1;
    data.screenWidth = -1;
}