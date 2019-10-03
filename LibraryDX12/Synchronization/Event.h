#pragma once

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/windows.h>
#include <chrono>

class Event : DXD::NonCopyable {
public:
    Event();
    explicit Event(BOOL initialValue);
    ~Event();
    Event(Event &&other) noexcept;
    Event &operator=(Event &&other) noexcept;

    void wait();
    void wait(DWORD milliseconds);
    void wait(std::chrono::milliseconds duration);

    HANDLE getHandle() const { return handle; }

protected:
    HANDLE handle;
};
