#pragma once

#include "DXD/ExternalHeadersWrappers/windows.h"
#include <d3d12.h>
#include <wrl.h>

using ID3D12CommandQueuePtr = Microsoft::WRL::ComPtr<ID3D12CommandQueue>;
using ID3D12DevicePtr = Microsoft::WRL::ComPtr<ID3D12Device3>;
using ID3D12DescriptorHeapPtr = Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>;
using ID3D12ResourcePtr = Microsoft::WRL::ComPtr<ID3D12Resource>;
using ID3D12PageablePtr = Microsoft::WRL::ComPtr<ID3D12Pageable>;
using ID3D12CommandAllocatorPtr = Microsoft::WRL::ComPtr<ID3D12CommandAllocator>;
using ID3D12GraphicsCommandListPtr = Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>;
using ID3D12FencePtr = Microsoft::WRL::ComPtr<ID3D12Fence1>;
using ID3D12DebugPtr = Microsoft::WRL::ComPtr<ID3D12Debug>;
using ID3D12InfoQueuePtr = Microsoft::WRL::ComPtr<ID3D12InfoQueue>;
using ID3D12RootSignaturePtr = Microsoft::WRL::ComPtr<ID3D12RootSignature>;
using ID3D12PipelineStatePtr = Microsoft::WRL::ComPtr<ID3D12PipelineState>;
