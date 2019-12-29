#include "ApplicationImpl.h"

#include "Utility/ThrowIfFailed.h"

#include <cassert>

namespace DXD {
std::unique_ptr<Application> Application::create(bool debugLayer, bool debugShaders, MinimizeBehavior minimizeBehavior) {
    assert(ApplicationImpl::instance == nullptr);
    return std::unique_ptr<Application>{new ApplicationImpl(debugLayer, debugShaders, minimizeBehavior)};
}
} // namespace DXD

ApplicationImpl *ApplicationImpl::instance = nullptr;

ApplicationImpl::ApplicationImpl(bool debugLayer, bool debugShaders, MinimizeBehavior minimizeBehavior)
    : debugLayerEnabled(enableDebugLayer(debugLayer)),
      debugShadersEnabled(debugShaders),
      minimizeBehavior(minimizeBehavior),
      factory(createFactory(debugLayerEnabled)),
      adapter(createAdapter(factory, false)),
      device(createDevice(adapter, debugLayerEnabled)),
      pipelineStateController(device),
      descriptorController(device),
      copyCommandQueue(device, D3D12_COMMAND_LIST_TYPE_COPY),
      directCommandQueue(device, D3D12_COMMAND_LIST_TYPE_DIRECT),
      backgroundWorkerController() {
    instance = this;
    pipelineStateController.compileAll();
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
}

ApplicationImpl::~ApplicationImpl() {
    CoUninitialize();
    instance = nullptr;
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

void ApplicationImpl::flushAllQueues() {
    const std::unique_lock<std::mutex> locks[] = {
        copyCommandQueue.getLock(true),
        directCommandQueue.getLock(true),
    };

    copyCommandQueue.flush(false);
    directCommandQueue.flush(false);
}

void ApplicationImpl::flushAllResources() {
    const std::unique_lock<std::mutex> locks[] = {
        copyCommandQueue.getLock(true),
        directCommandQueue.getLock(true),
    };

    copyCommandQueue.performResourcesDeletion(false);
    directCommandQueue.performResourcesDeletion(false);
}

D2DContext &ApplicationImpl::getD2DContext() {
    struct Tag {};
    static const auto create = [&]() { return new D2DContext(device, directCommandQueue.getCommandQueue()); };
    return LazyLoadHelper::getLazy<Tag>(this->d2dContext, create);
}

bool ApplicationImpl::isD2DContextInitialized() {
    return this->d2dContext != nullptr;
}
