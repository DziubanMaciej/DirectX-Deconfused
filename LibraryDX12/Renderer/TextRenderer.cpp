#include "TextRenderer.h"

#include "Application/ApplicationImpl.h"
#include "Resource/D2DWrappedResource.h"
#include "Scene/SceneImpl.h"
#include "Scene/TextImpl.h"
#include "Utility/ThrowIfFailed.h"
#include "Window/SwapChain.h"

TextRenderer::TextRenderer(SceneImpl &scene)
    : scene(scene) {}

void TextRenderer::renderTexts(D2DWrappedResource &output) {
    if (!isEnabled()) {
        return;
    }

    // Get the contexts
    auto &d2dDeviceContext = ApplicationImpl::getInstance().getD2DContext().getD2DDeviceContext();
    auto &d3d11DeviceContext = ApplicationImpl::getInstance().getD2DContext().getD3D11DeviceContext();

    // Acquire D2D back buffer. Acquired buffer is automatically released by the destructor and flush is made
    AcquiredD2DWrappedResource backBuffer = output.acquire();
    ID2D1BitmapPtr &d2dBackBuffer = backBuffer.getD2DResource();

    // Render text directly to the back buffer.
    const D2D1_SIZE_F rtSize = d2dBackBuffer->GetSize();
    const D2D1_RECT_F textRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);
    d2dDeviceContext->SetTarget(d2dBackBuffer.Get());
    d2dDeviceContext->BeginDraw();
    d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
    for (auto text : scene.getTexts()) {
        text->update();
        text->draw(textRect);
    }
    throwIfFailed(d2dDeviceContext->EndDraw());
}

bool TextRenderer::isEnabled() const {
    return scene.getTexts().size() > 0u;
}
