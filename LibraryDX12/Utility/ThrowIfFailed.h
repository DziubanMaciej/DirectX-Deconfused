#pragma once

#include "DXD/Logger.h"

#include "DXD/ExternalHeadersWrappers/d3dcompiler.h"
#include "DXD/ExternalHeadersWrappers/windows.h"
#include <exception>

inline void throwIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        const auto lastError = GetLastError();
        abort();
    }
}

inline void throwIfFailed(HRESULT hr, ID3DBlob *blob) {
    if (blob != nullptr) {
        DXD::log(static_cast<char *>(blob->GetBufferPointer()));
    }
    throwIfFailed(hr);
}

#define UNREACHABLE_CODE() abort();
