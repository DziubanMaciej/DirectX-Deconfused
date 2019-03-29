#pragma once

#include "DXD/ExternalHeadersWrappers/windows.h"
#include <wrl.h>
#include <dxgi1_6.h>

using IDXGIFactoryPtr = Microsoft::WRL::ComPtr<IDXGIFactory6>;
using IDXGIAdapterPtr = Microsoft::WRL::ComPtr<IDXGIAdapter4>;
using IDXGISwapChainPtr = Microsoft::WRL::ComPtr<IDXGISwapChain4>;
