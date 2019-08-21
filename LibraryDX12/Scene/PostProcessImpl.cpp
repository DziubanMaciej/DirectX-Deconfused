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

    this->dataBlackBars = std::make_unique<DataBlackBars>();
    this->dataBlackBars->screenHeight = -1;
    this->dataBlackBars->screenWidth = -1;
    this->dataBlackBars->leftMarginPercent = leftMarginPercent;
    this->dataBlackBars->rightMarginPercent = rightMarginPercent;
    this->dataBlackBars->topMarginPercent = topMarginPercent;
    this->dataBlackBars->bottomMarginPercent = bottomMarginPercent;
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

    this->dataConvolution = std::make_unique<DataConvolution>();
    this->dataConvolution->screenHeight = -1;
    this->dataConvolution->screenWidth = -1;
    this->dataConvolution->divider = divider;
    this->dataConvolution->kernel = XMFLOAT4X3(a00, a01, a02, 0.f,
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

    this->dataLinearColorCorrection = std::make_unique<PostProcessLinearColorCorrectionCB>();
    this->dataLinearColorCorrection->screenWidth = -1;
    this->dataLinearColorCorrection->screenHeight = -1;
    this->dataLinearColorCorrection->colorMatrix = XMFLOAT4X3(a00, a01, a02, 0.f,
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
