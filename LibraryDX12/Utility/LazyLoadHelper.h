#pragma once

#include <memory>
#include <mutex>

/// \brief Thread safe implementation of lazy creation of arbitrary objects
struct LazyLoadHelper {
    /// Function simply retrieves object from pointer if it's already created
    /// or first creates it if the pointer is empty.
    /// \tparam LazyLoadedObject type of lazy loaded object
    /// \tparam CreateFunction callback type for object creation
    /// \tparam TagType additional type used to distinguish lazy loading the objects
    /// of the same class in multiple contexts
    /// \param objectPtr pointer storing the object
    /// \param createFunction callback for object creation
    /// \return object store in objectPtr or newly created instance
    template <typename TagType, typename LazyLoadedObject, typename CreateFunction>
    static LazyLoadedObject &getLazy(std::unique_ptr<LazyLoadedObject> &objectPtr, CreateFunction createFunction) {
        if (objectPtr == nullptr) {
            // Critical section
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock{mutex};

            // Check again to avoid race conditions
            if (objectPtr == nullptr) {
                objectPtr.reset(createFunction());
            }
        }
        return *objectPtr;
    }
};
