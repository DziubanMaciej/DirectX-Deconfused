#pragma once

#include "Application/SettingsImpl.h"
#include "CommandList/CommandQueue.h"
#include "Descriptor/DescriptorController.h"
#include "PipelineState/PipelineStateController.h"
#include "Threading/BackgroundWorkerController.h"

#include "DXD/Application.h"

#include "DXD/ExternalHeadersWrappers/dxgi.h"


#include <d3d11on12.h>
#include <d3d11.h>
#include <d2d1_3.h>
#include <dwrite.h>

class ApplicationImpl : public DXD::Application {
protected:
    friend class DXD::Application;
    ApplicationImpl(bool debugLayer);

public:
    ~ApplicationImpl() override;

    // API accessors
    void setCallbackHandler(DXD::CallbackHandler *arg) override { this->callbackHandler = arg; }
    DXD::Settings &getSettings() override { return settings; }

    // Flushing all work
    void flushAllQueues();
    void flushAllResources();

    // Internal getters
    static auto &getInstance() { return *instance; }
    auto &getSettingsImpl() { return settings; }
    auto getCallbackHandler() const { return callbackHandler; }
    auto getFactory() { return factory; }
    auto getAdapter() { return adapter; }
    auto getDevice() { return device; }
    auto &getPipelineStateController() { return pipelineStateController; }
    auto &getDescriptorController() { return descriptorController; }
    auto &getBackgroundWorkerController() { return backgroundWorkerController; }
    auto &getDirectCommandQueue() { return directCommandQueue; }
    auto &getCopyCommandQueue() { return copyCommandQueue; }

protected:
    // Creation helpers
    static bool enableDebugLayer(bool debugLayer);
    static IDXGIFactoryPtr createFactory(bool debugLayer);
    static IDXGIAdapterPtr createAdapter(IDXGIFactoryPtr factory, bool useWarp);
    static ID3D12DevicePtr createDevice(IDXGIAdapterPtr &adapter, bool debugLayer);

    // Singleton instance
    static ApplicationImpl *instance;

    // DX12 context
    const bool debugLayerEnabled;
    IDXGIFactoryPtr factory;
    IDXGIAdapterPtr adapter;
    ID3D12DevicePtr device;

    // API objects
    SettingsImpl settings = {};
    DXD::CallbackHandler *callbackHandler = {};

    // Internal objects used globally
    PipelineStateController pipelineStateController;
    DescriptorController descriptorController;
    CommandQueue copyCommandQueue;
    CommandQueue directCommandQueue;
    BackgroundWorkerController backgroundWorkerController;

public:
    void createD3D11Device();
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
    Microsoft::WRL::ComPtr<ID3D11On12Device> m_d3d11On12Device;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dWriteFactory;
    Microsoft::WRL::ComPtr<ID2D1Factory3> m_d2dFactory;
    Microsoft::WRL::ComPtr<ID2D1Device2> m_d2dDevice;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;

};
