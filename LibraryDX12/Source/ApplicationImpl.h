#pragma once

#include "DXD/Application.h"
#include "Source/CommandQueue.h"
#include "DXD/ExternalHeadersWrappers/dxgi.h"

class ApplicationImpl : public DXD::Application {
protected:
    friend class DXD::Application;
    ApplicationImpl(bool debugLayer);

public:
    void setCallbackHandler(DXD::CallbackHandler *callbackHandler) override;
    DXD::CallbackHandler *getCallbackHandler() const;

    auto getFactory() { return factory; }
    auto getAdapter() { return adapter; }
    auto getDevice() { return device; }

protected:
    static IDXGIFactoryPtr createFactory(bool debugLayer);
    static IDXGIAdapterPtr createAdapter(IDXGIFactoryPtr factory, bool useWarp);
    static ID3D12DevicePtr createDevice(IDXGIAdapterPtr &adapter, bool debugLayer);

    DXD::CallbackHandler *callbackHandler;
    IDXGIFactoryPtr factory;
    IDXGIAdapterPtr adapter;
    ID3D12DevicePtr device;
    CommandQueue copyCommandQueue;
    CommandQueue directCommandQueue;
};
