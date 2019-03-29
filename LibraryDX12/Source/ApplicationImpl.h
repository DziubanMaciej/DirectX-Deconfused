#pragma once

#include "DXD/Application.h"
#include "DXD/ExternalHeadersWrappers/dxgi.h"

class ApplicationImpl : public Application {
protected:
    friend class Application;
    ApplicationImpl(bool debugLayer);

public:
    void setCallbackHandler(CallbackHandler *callbackHandler) override;
    CallbackHandler *getCallbackHandler() const;

    auto getFactory() { return factory; }
    auto getAdapter() { return adapter; }
    auto getDevice() { return device; }

protected:
    static IDXGIFactoryPtr createFactory(bool debugLayer);
    static IDXGIAdapterPtr createAdapter(IDXGIFactoryPtr factory, bool useWarp);
    static ID3D12DevicePtr createDevice(IDXGIAdapterPtr &adapter, bool debugLayer);

    CallbackHandler *callbackHandler;
    IDXGIFactoryPtr factory;
    IDXGIAdapterPtr adapter;
    ID3D12DevicePtr device;
};