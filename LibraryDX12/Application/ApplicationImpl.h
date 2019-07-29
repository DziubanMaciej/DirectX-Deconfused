#pragma once

#include "CommandList/CommandQueue.h"
#include "Descriptor/DescriptorManager.h"
#include "PipelineState/PipelineStateController.h"

#include "DXD/Application.h"

#include "DXD/ExternalHeadersWrappers/dxgi.h"

class ApplicationImpl : public DXD::Application {
protected:
    friend class DXD::Application;
    ApplicationImpl(bool debugLayer);

public:
    void setCallbackHandler(DXD::CallbackHandler *callbackHandler) override;
    DXD::CallbackHandler *getCallbackHandler() const;

    void flushAllQueues();
    void flushAllResources();

    auto getFactory() { return factory; }
    auto getAdapter() { return adapter; }
    auto getDevice() { return device; }
    auto &getPipelineStateController() { return pipelineStateController; }
    auto &getDescriptorController() { return descriptorManager; }
    auto &getDirectCommandQueue() { return directCommandQueue; }
    auto &getCopyCommandQueue() { return copyCommandQueue; }

protected:
    static bool enableDebugLayer(bool debugLayer);
    static IDXGIFactoryPtr createFactory(bool debugLayer);
    static IDXGIAdapterPtr createAdapter(IDXGIFactoryPtr factory, bool useWarp);
    static ID3D12DevicePtr createDevice(IDXGIAdapterPtr &adapter, bool debugLayer);

    const bool debugLayerEnabled;
    DXD::CallbackHandler *callbackHandler;
    IDXGIFactoryPtr factory;
    IDXGIAdapterPtr adapter;
    ID3D12DevicePtr device;
    PipelineStateController pipelineStateController;
    DescriptorManager descriptorManager;
    CommandQueue copyCommandQueue;
    CommandQueue directCommandQueue;
};
