#include "Event.h"

#include <cassert>

Event::Event() : Event(false) {}

Event::Event(BOOL initialValue) : handle(::CreateEvent(NULL, false, initialValue, NULL)) {
    assert(handle != NULL);
}

Event::~Event() {
    if (handle != NULL) {
        ::CloseHandle(handle);
    }
}

Event::Event(Event &&other) noexcept {
    *this = std::move(other);
}

Event &Event::operator=(Event &&other) noexcept {
    this->handle = other.handle;
    other.handle = nullptr;
    return *this;
}

void Event::wait() const {
    wait(INFINITE);
}

void Event::wait(DWORD milliseconds) const {
    const auto result = ::WaitForSingleObject(this->handle, milliseconds);
    assert(result == 0);
}

void Event::wait(std::chrono::milliseconds duration) const {
    wait(static_cast<DWORD>(duration.count()));
}

void Event::signal() {
    ::SetEvent(this->handle);
}
