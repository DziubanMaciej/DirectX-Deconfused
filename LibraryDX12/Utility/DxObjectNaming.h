#pragma once

#include "CommandList/CommandList.h"
#include "Resource/Resource.h"

#include <DXD/ExternalHeadersWrappers/d3d12.h>
#include <string>

#if defined(_DEBUG)
template <typename T>
inline void setObjectName(const Microsoft::WRL::ComPtr<T> &object, const wchar_t *name) {
    object->SetName(name);
}

template <typename T, typename... Args>
inline void setObjectName(const Microsoft::WRL::ComPtr<T> &object, const wchar_t *format, Args &&... args) {
    wchar_t buffer[1000];
    swprintf_s(buffer, format, std::forward<Args>(args)...);
    object->SetName(buffer);
}

template <typename... Args>
inline void setObjectName(const Resource &resource, Args &&... args) {
    setObjectName(resource.getResource(), std::forward<Args>(args)...);
}

template <typename... Args>
inline void setObjectName(CommandList &commandList, Args &&... args) {
    setObjectName(commandList.getCommandList(), std::forward<Args>(args)...);
}

#define SET_OBJECT_NAME(...) setObjectName(__VA_ARGS__);

#else
#define SET_OBJECT_NAME(...)
#endif
