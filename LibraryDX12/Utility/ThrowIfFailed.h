#pragma once

#include "DXD/ExternalHeadersWrappers/windows.h"
#include <exception>

inline void throwIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        const auto lastError = GetLastError();
        throw std::exception();
    }
}
