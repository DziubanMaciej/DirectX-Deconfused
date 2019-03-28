#pragma once

#include <wrl.h>

template <typename T>
size_t getComPtrRefCount(Microsoft::WRL::ComPtr<T> &ptr) {
    ptr->AddRef();
    return ptr->Release();
}
