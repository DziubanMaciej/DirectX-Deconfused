#include "PostProcessRenderer.h"

#include "CommandList/CommandList.h"
#include "Renderer/RenderData.h"
#include "Scene/PostProcessImpl.h"
#include "Scene/SceneImpl.h"
#include "Utility/AlternatingResources.h"
#include "Utility/ThrowIfFailed.h"
#include "Window/SwapChain.h"

PostProcessRenderer::PostProcessRenderer(RenderData &renderData, const std::vector<PostProcessImpl *> &postProcesses, float screenWidth, float screenHeight)
    : renderData(renderData),
      postProcesses(postProcesses),
      screenWidth(screenWidth),
      screenHeight(screenHeight) {}

// ------------------------------------------------------------------------------------- Rendering

void PostProcessRenderer::renderPostProcesses(CommandList &commandList, AlternatingResources &alternatingResources, float screenWidth, float screenHeight) {
    commandList.RSSetViewport(0.f, 0.f, screenWidth, screenHeight);
    commandList.RSSetScissorRectNoScissor();
    commandList.IASetPrimitiveTopologyTriangleList();

    size_t postProcessIndex = 0u;
    Resource *source{}, *destination{};
    for (PostProcessImpl *postProcess : postProcesses) {
        if (!postProcess->isEnabled()) {
            continue;
        }

        // Render
        renderPostProcess(*postProcess, commandList, nullptr, alternatingResources, screenWidth, screenHeight);
        alternatingResources.swapResources();
        postProcessIndex++;
    }
}

void PostProcessRenderer::renderPostProcess(PostProcessImpl &postProcess, CommandList &commandList, Resource *optionalInput,
                                            AlternatingResources &alternatingResources, float screenWidth, float screenHeight) {
    assert(postProcess.isEnabled());
    Resource *source{}, *destination{};

    // Render post process base on its type
    if (postProcess.getType() == PostProcessImpl::Type::CONVOLUTION) {
        getSourceAndDestinationForPostProcess(alternatingResources, optionalInput, source, destination);
        prepareSourceAndDestinationForPostProcess(commandList, *source, *destination, false);

        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().convolution;
        postProcessData.screenWidth = screenWidth;
        postProcessData.screenHeight = screenHeight;

        // Set state
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_CONVOLUTION);
        commandList.setRoot32BitConstant(0, postProcessData);
        commandList.setSrvInDescriptorTable(1, 0, *source);
        commandList.OMSetRenderTargetNoDepth(*destination);
        commandList.IASetVertexBuffer(renderData.getFullscreenVB());

        // Render
        commandList.draw(6u);
    } else if (postProcess.getType() == PostProcessImpl::Type::BLACK_BARS) {
        getSourceAndDestinationForPostProcess(alternatingResources, optionalInput, source, destination);
        prepareSourceAndDestinationForPostProcess(commandList, *source, *destination, false);

        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().blackBars;
        postProcessData.screenWidth = screenWidth;
        postProcessData.screenHeight = screenHeight;

        // Set state
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_BLACK_BARS);
        commandList.setRoot32BitConstant(0, postProcessData);
        commandList.setSrvInDescriptorTable(1, 0, *source);
        commandList.OMSetRenderTargetNoDepth(*destination);
        commandList.IASetVertexBuffer(renderData.getFullscreenVB());

        // Render
        commandList.draw(6u);
    } else if (postProcess.getType() == PostProcessImpl::Type::LINEAR_COLOR_CORRECTION) {
        getSourceAndDestinationForPostProcess(alternatingResources, optionalInput, source, destination);
        prepareSourceAndDestinationForPostProcess(commandList, *source, *destination, false);

        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().linearColorCorrection;
        postProcessData.screenWidth = screenWidth;
        postProcessData.screenHeight = screenHeight;

        // Set state
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_LINEAR_COLOR_CORRECTION);
        commandList.setRoot32BitConstant(0, postProcessData);
        commandList.setSrvInDescriptorTable(1, 0, *source);
        commandList.OMSetRenderTargetNoDepth(*destination);
        commandList.IASetVertexBuffer(renderData.getFullscreenVB());

        // Render
        commandList.draw(6u);
    } else if (postProcess.getType() == PostProcessImpl::Type::GAUSSIAN_BLUR) {
        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().gaussianBlur;
        postProcessData.cb.screenWidth = screenWidth;
        postProcessData.cb.screenHeight = screenHeight;

        const UINT threadGroupCountX = static_cast<UINT>(screenWidth / 16 + 0.5f);
        const UINT threadGroupCountY = static_cast<UINT>(screenHeight / 16 + 0.5f);

        // Set cross-pass state
        commandList.IASetVertexBuffer(renderData.getFullscreenVB());

        // Render all passes of the blur filter
        for (auto passIndex = 0u; passIndex < postProcessData.passCount; passIndex++) {
            // Render horizontal pass
            getSourceAndDestinationForPostProcess(alternatingResources, optionalInput, source, destination);
            prepareSourceAndDestinationForPostProcess(commandList, *source, *destination, true);
            commandList.setPipelineStateAndComputeRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR_COMPUTE_HORIZONTAL);
            commandList.setSrvInDescriptorTable(0, 0, *source);
            commandList.setUavInDescriptorTable(0, 1, *destination);
            commandList.setRoot32BitConstant(1, postProcessData.cb);
            commandList.dispatch(threadGroupCountX, threadGroupCountY, 1u);
            alternatingResources.swapResources();

            // Clear optional input so rest of the passes are done in ping-pong manner
            optionalInput = nullptr;

            // Render vertical pass
            getSourceAndDestinationForPostProcess(alternatingResources, optionalInput, source, destination);
            prepareSourceAndDestinationForPostProcess(commandList, *source, *destination, true);
            commandList.setPipelineStateAndComputeRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR_COMPUTE_VERTICAL);
            commandList.setSrvInDescriptorTable(0, 0, *source);
            commandList.setUavInDescriptorTable(0, 1, *destination);
            commandList.setRoot32BitConstant(1, postProcessData.cb);
            commandList.dispatch(threadGroupCountX, threadGroupCountY, 1u);

            const bool lastPass = (passIndex == postProcessData.passCount - 1);
            if (!lastPass) {
                alternatingResources.swapResources();
            }
        }
    } else if (postProcess.getType() == PostProcessImpl::Type::FXAA) {
        getSourceAndDestinationForPostProcess(alternatingResources, optionalInput, source, destination);
        prepareSourceAndDestinationForPostProcess(commandList, *source, *destination, false);

        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().fxaa;
        postProcessData.screenWidth = screenWidth;
        postProcessData.screenHeight = screenHeight;

        // Set state
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_FXAA);
        commandList.setRoot32BitConstant(0, postProcessData);
        commandList.setSrvInDescriptorTable(1, 0, *source);
        commandList.OMSetRenderTargetNoDepth(*destination);
        commandList.IASetVertexBuffer(renderData.getFullscreenVB());

        // Render
        commandList.draw(6u);
    } else if (postProcess.getType() == PostProcessImpl::Type::GAMMA_CORRECTION) {
        getSourceAndDestinationForPostProcess(alternatingResources, optionalInput, source, destination);
        prepareSourceAndDestinationForPostProcess(commandList, *source, *destination, false);

        // Prepare constant buffer
        auto &postProcessData = postProcess.getData().gammaCorrection;
        postProcessData.screenWidth = screenWidth;
        postProcessData.screenHeight = screenHeight;

        // Set state
        commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_GAMMA_CORRECTION);
        commandList.setRoot32BitConstant(0, postProcessData);
        commandList.setSrvInDescriptorTable(1, 0, alternatingResources.getSource());
        commandList.OMSetRenderTargetNoDepth(*destination);
        commandList.IASetVertexBuffer(renderData.getFullscreenVB());

        // Render
        commandList.draw(6u);
    } else {
        UNREACHABLE_CODE();
    }
}

void PostProcessRenderer::renderBloom(CommandList &commandList, Resource &input, Resource &output) {
    // Blur the bloom map
    AlternatingResources &alternatingResourcesForBlur = renderData.getHelperAlternatingResources();
    renderPostProcess(renderData.getPostProcessForBloom(), commandList,
                      &input, alternatingResourcesForBlur, screenWidth, screenHeight);

    // Apply bloom on output buffer
    Resource &blurredBloomMap = alternatingResourcesForBlur.getDestination();
    commandList.transitionBarrier(blurredBloomMap, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.transitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.setPipelineStateAndGraphicsRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_POST_PROCESS_APPLY_BLOOM);
    PostProcessApplyBloomCB cb = {};
    cb.screenWidth = screenWidth;
    cb.screenHeight = screenHeight;
    commandList.setRoot32BitConstant(0, cb);
    commandList.setSrvInDescriptorTable(1, 0, blurredBloomMap);
    commandList.OMSetRenderTargetNoDepth(output);
    commandList.IASetVertexBuffer(renderData.getFullscreenVB());
    commandList.draw(6u);
}

// ------------------------------------------------------------------------------------- Helpers

void PostProcessRenderer::getSourceAndDestinationForPostProcess(AlternatingResources &alternatingResources, Resource *optionalInput,
                                                                Resource *&outSource, Resource *&outDestination) {
    outSource = (optionalInput != nullptr) ? optionalInput : &alternatingResources.getSource();
    outDestination = &alternatingResources.getDestination();
}

void PostProcessRenderer::prepareSourceAndDestinationForPostProcess(CommandList &commandList, Resource &source, Resource &destination, bool compute) {
    if (compute) {
        commandList.transitionBarrier(source, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        commandList.transitionBarrier(destination, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    } else {
        commandList.transitionBarrier(source, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        commandList.transitionBarrier(destination, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
}
