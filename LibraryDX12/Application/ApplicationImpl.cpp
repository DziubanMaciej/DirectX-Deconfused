#include "ApplicationImpl.h"

#include "Utility/ThrowIfFailed.h"

// ------------------------------------------------------------- Creating

namespace DXD {
std::unique_ptr<Application> Application::create(bool debugLayer) {
    return std::unique_ptr<Application>{new ApplicationImpl(debugLayer)};
}
} // namespace DXD

ApplicationImpl::ApplicationImpl(bool debugLayer) : debugLayerEnabled(enableDebugLayer(debugLayer)),
                                                    factory(createFactory(debugLayerEnabled)),
                                                    adapter(createAdapter(factory, false)),
                                                    device(createDevice(adapter, debugLayerEnabled)),
                                                    pipelineStateController(device),
                                                    descriptorManager(device),
                                                    copyCommandQueue(device, D3D12_COMMAND_LIST_TYPE_COPY),
                                                    directCommandQueue(device, D3D12_COMMAND_LIST_TYPE_DIRECT) {
    pipelineStateController.compileAll();
}

bool ApplicationImpl::enableDebugLayer(bool debugLayer) {
    if (debugLayer) {
        ID3D12DebugPtr debugInterface;
        const auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
        if (SUCCEEDED(result)) {
            DXD::log("Debug layer enabling SUCCESS\n");
            debugInterface->EnableDebugLayer();
            return true;
        } else {
            DXD::log("Debug layer enabling FAILED\n");
            return false;
        }
    }
    return false;
}

IDXGIFactoryPtr ApplicationImpl::createFactory(bool debugLayer) {
    IDXGIFactoryPtr factory;
    UINT createFactoryFlags = debugLayer ? DXGI_CREATE_FACTORY_DEBUG : 0u;
    throwIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory)));
    return factory;
}

IDXGIAdapterPtr ApplicationImpl::createAdapter(IDXGIFactoryPtr factory, bool useWarp) {
    // Get IDXGIAdapter1 from factory
    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter1;
    if (useWarp) {
        throwIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter1)));
    } else {
        throwIfFailed(factory->EnumAdapters1(0u, &adapter1)); // TODO we can select different adapters here
    }

    // Cast to proper adapter version
    IDXGIAdapterPtr adapter;
    throwIfFailed(adapter1.As(&adapter));
    return adapter;
}

ID3D12DevicePtr ApplicationImpl::createDevice(IDXGIAdapterPtr &adapter, bool debugLayer) {
    ID3D12DevicePtr device;
    throwIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
    if (debugLayer) {
        ID3D12InfoQueuePtr pInfoQueue;
        if (SUCCEEDED(device.As(&pInfoQueue))) {
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

            D3D12_MESSAGE_CATEGORY *deniedCategories = nullptr;
            D3D12_MESSAGE_SEVERITY deniedSeverities[] = {D3D12_MESSAGE_SEVERITY_INFO};
            D3D12_MESSAGE_ID deniedIds[] = {
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, // I'm really not sure how to avoid this message.
                D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
                D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                     // This warning occurs when using capture frame while graphics debugging.
            };

            D3D12_INFO_QUEUE_FILTER NewFilter = {};
            NewFilter.DenyList.NumCategories = 0u;
            NewFilter.DenyList.pCategoryList = deniedCategories;
            NewFilter.DenyList.NumSeverities = _countof(deniedSeverities);
            NewFilter.DenyList.pSeverityList = deniedSeverities;
            NewFilter.DenyList.NumIDs = _countof(deniedIds);
            NewFilter.DenyList.pIDList = deniedIds;

            throwIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
        }
    }
    return device;
}

// ------------------------------------------------------------- Accessors

void ApplicationImpl::setCallbackHandler(DXD::CallbackHandler *callbackHandler) {
    this->callbackHandler = callbackHandler;
}

DXD::CallbackHandler *ApplicationImpl::getCallbackHandler() const {
    return this->callbackHandler;
}

void ApplicationImpl::flushAllQueues() {
    copyCommandQueue.flush();
    directCommandQueue.flush();
}

void ApplicationImpl::flushAllResources() {
    copyCommandQueue.performResourcesDeletion();
    directCommandQueue.performResourcesDeletion();
}