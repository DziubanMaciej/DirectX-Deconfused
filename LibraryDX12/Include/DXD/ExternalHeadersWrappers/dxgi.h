#pragma once

#include "DXD/ExternalHeadersWrappers/windows.h"
#include <dxgi1_6.h>
#include <wrl.h>

using IDXGIFactoryPtr = Microsoft::WRL::ComPtr<IDXGIFactory5>;
using IDXGIAdapterPtr = Microsoft::WRL::ComPtr<IDXGIAdapter4>;
using IDXGISwapChainPtr = Microsoft::WRL::ComPtr<IDXGISwapChain4>;
