#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <memory>

namespace DXD {

/// \brief Holds asynchronous operation information
///
/// Event objects serve two purposes. They allow checking state of an asynchronous operation
/// performed by the engine (both blocking and polling) and they provide additional information
/// when operation is complete, e.g. the return code. Event class should not be used directly.
/// Although the interface is generic, applications should use only specializations provided
/// by the library for each asynchronous operation, such as ObjLoadEvent.
template <typename Data>
class EXPORT Event : NonCopyableAndMovable {
public:
    /// Sets the operation-specific data, marks the Event as complete and unblocks all threads
    /// waiting on this event. This method is called internally by the engine and typically
    /// there's no need for the applications to use it.
    /// \param data piece of data to set
    virtual void signal(const Data &data) = 0;

    /// Checks completion status of operation and returns immediately. Completion should be verified
    /// before calling getData.
    /// \return true if associated operation has ended
    virtual bool isComplete() const = 0;

    /// Retrieves data set by signal and returns to the caller immediately. Calling this method
    /// before the operation has actually ended is undefined and may lead to failures. Application
    /// should check completion status using the isComplete method.
    /// \return operation-specific data
    virtual const Data &getData() const = 0;

    /// Blocks the calling thread until the asynchronous operation completes or returns immediately
    /// if it's already completed. Returns the same result getData would return.
    /// \return operation-specific data
    virtual const Data &wait() const = 0;

    /// Factory function used to create instances of Event
    /// \return created instance
    static std::unique_ptr<Event> create();
    virtual ~Event() = default;

protected:
    Event() = default;
};

} // namespace DXD
