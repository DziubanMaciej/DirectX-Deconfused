#pragma once

#include <wrl.h>

template <typename T>
size_t getComPtrRefCount(Microsoft::WRL::ComPtr<T> &ptr) {
    ptr.Get()->AddRef();
    return ptr.Get()->Release();
}
