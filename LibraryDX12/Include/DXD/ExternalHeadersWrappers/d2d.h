#pragma once

// Has to be the first include
#include <DXD/ExternalHeadersWrappers/windows.h>
#include <wrl.h>

// Then the libraries
#include <d2d1_3.h>
#include <d3d11.h>
#include <d3d11on12.h>
#include <dwrite.h>

// DX11
using ID3D11DevicePtr = Microsoft::WRL::ComPtr<ID3D11Device>;
using ID3D11ResourcePtr = Microsoft::WRL::ComPtr<ID3D11Resource>;
using ID3D11DeviceContextPtr = Microsoft::WRL::ComPtr<ID3D11DeviceContext>;
using ID3D11On12DevicePtr = Microsoft::WRL::ComPtr<ID3D11On12Device>;

// D2D
using ID2D1DevicePtr = Microsoft::WRL::ComPtr<ID2D1Device2>;
using ID2D1BitmapPtr = Microsoft::WRL::ComPtr<ID2D1Bitmap1>;
using ID2D1FactoryPtr = Microsoft::WRL::ComPtr<ID2D1Factory3>;
using ID2D1DeviceContextPtr = Microsoft::WRL::ComPtr<ID2D1DeviceContext2>;
using ID2D1SolidColorBrushPtr = Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>;

// DWrite
using IDWriteFactoryPtr = Microsoft::WRL::ComPtr<IDWriteFactory>;
using IDWriteTextFormatPtr = Microsoft::WRL::ComPtr<IDWriteTextFormat>;
