#pragma once

#include "DXD/ExternalHeadersWrappers/d3dcompiler.h"
#include "DXD/ExternalHeadersWrappers/windows.h"
#include <exception>

inline void throwIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        const auto lastError = GetLastError();
        throw std::exception();
    }
}

inline void throwIfFailed(HRESULT hr, ID3DBlob *blob) {
    if (blob != nullptr) {
        OutputDebugStringA(static_cast<char *>(blob->GetBufferPointer()));
    }
    throwIfFailed(hr);
}
