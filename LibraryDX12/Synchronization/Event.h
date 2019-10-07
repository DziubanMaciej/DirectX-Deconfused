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

    void wait() const;
    void wait(DWORD milliseconds) const;
    void wait(std::chrono::milliseconds duration) const;

    HANDLE getHandle() const { return handle; }

protected:
    HANDLE handle;
};
