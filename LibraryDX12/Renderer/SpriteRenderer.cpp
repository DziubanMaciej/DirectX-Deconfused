#include "SpriteRenderer.h"

#include "ConstantBuffers/ConstantBuffers.h"
#include "Renderer/RenderData.h"
#include "Resource/TextureImpl.h"
#include "Scene/SceneImpl.h"
#include "Scene/SpriteImpl.h"
#include "Window/SwapChain.h"

SpriteRenderer::SpriteRenderer(SwapChain &swapChain, RenderData &renderData, SceneImpl &scene)
    : swapChain(swapChain),
      renderData(renderData),
      scene(scene) {}

void SpriteRenderer::renderSprites(CommandList &commandList, Resource &output) {
    commandList.RSSetViewport(0.f, 0.f, swapChain.getWidth(), swapChain.getHeight());
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_SPRITE);
    commandList.OMSetRenderTargetNoDepth(output);
    commandList.IASetVertexBuffer(renderData.getFullscreenVB());

    for (auto &sprite : scene.getSprites()) {
        renderSprite(commandList, output, sprite);
    }
}

void SpriteRenderer::renderSprite(CommandList &commandList, Resource &output, SpriteImpl *sprite) {
    // Prepare texture
    const TextureImpl &texture = sprite->getTextureImpl();

    // Prepare constant buffer
    SpriteCB spriteData = sprite->getData();
    spriteData.screenWidth = swapChain.getWidth();
    spriteData.screenHeight = swapChain.getHeight();

    // Render
    commandList.setRoot32BitConstant(0, spriteData);
    commandList.setSrvInDescriptorTable(1, 0, texture);
    commandList.draw(6u);
}
