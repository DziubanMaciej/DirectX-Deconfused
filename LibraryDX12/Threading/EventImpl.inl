#include "EventImpl.h"

namespace DXD {
    template <typename Data>
    std::unique_ptr<Event<Data>> Event<Data>::create() {
        return std::unique_ptr<Event<Data>>(new EventImpl<Data>());
    }
}

