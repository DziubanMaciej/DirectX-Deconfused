#include "KernelEvent.h"

#include <cassert>

KernelEvent::KernelEvent() : KernelEvent(false) {}

KernelEvent::KernelEvent(BOOL initialValue) : handle(::CreateEvent(NULL, false, initialValue, NULL)) {
    assert(handle != NULL);
}

KernelEvent::~KernelEvent() {
    if (handle != NULL) {
        ::CloseHandle(handle);
    }
}

KernelEvent::KernelEvent(KernelEvent &&other) noexcept {
    *this = std::move(other);
}

KernelEvent &KernelEvent::operator=(KernelEvent &&other) noexcept {
    this->handle = other.handle;
    other.handle = nullptr;
    return *this;
}

void KernelEvent::wait() const {
    wait(INFINITE);
}

void KernelEvent::wait(DWORD milliseconds) const {
    const auto result = ::WaitForSingleObject(this->handle, milliseconds);
    assert(result == 0);
}

void KernelEvent::wait(std::chrono::milliseconds duration) const {
    wait(static_cast<DWORD>(duration.count()));
}
