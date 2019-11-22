#pragma once

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/windows.h>
#include <chrono>

class KernelEvent : DXD::NonCopyable {
public:
    KernelEvent();
    explicit KernelEvent(BOOL initialValue);
    ~KernelEvent();
    KernelEvent(KernelEvent &&other) noexcept;
    KernelEvent &operator=(KernelEvent &&other) noexcept;

    void wait() const;
    void wait(DWORD milliseconds) const;
    void wait(std::chrono::milliseconds duration) const;

    HANDLE getHandle() const { return handle; }

protected:
    HANDLE handle;
};
