#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"
#include "DXD/ExternalHeadersWrappers/windows.h"
#include <debugapi.h>
#include <mutex>
#include <string>

namespace DXD {

class EXPORT std::mutex;

class EXPORT LoggerData : NonCopyableAndMovable {
private:
    LoggerData() = delete;

    template <typename... Args>
    friend inline void log(const std::string &format, Args &&... args);

    static std::mutex mutex;
    static char buffer[4096];
};

template <typename... Args>
inline void log(const std::string &format, Args &&... args) {
    std::lock_guard<std::mutex> lock{LoggerData::mutex};
    sprintf_s(LoggerData::buffer, format.c_str(), std::forward<Args>(args)...);
    log(LoggerData::buffer);
}

inline void log(const std::string &message) {
    log(message.c_str());
}

inline void log(const char *message) {
    OutputDebugStringA((message));
}

} // namespace DXD
