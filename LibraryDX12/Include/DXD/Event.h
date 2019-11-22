#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <memory>

namespace DXD {

/// \brief Holds asynchronous operation information
template <typename Data>
class EXPORT Event : NonCopyableAndMovable {
public:
    virtual void signal(const Data &data) = 0;

    virtual bool isComplete() const = 0;

    virtual const Data &getData() const = 0;

    virtual const Data &wait() const = 0;

    static std::unique_ptr<Event> create();
    virtual ~Event() = default;

protected:
    Event() = default;
};

} // namespace DXD
