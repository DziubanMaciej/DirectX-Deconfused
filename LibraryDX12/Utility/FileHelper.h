#pragma once

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/windows.h"
#include <string>

struct FileHelper : DXD::NonCopyableAndMovable {
    FileHelper() = delete;

    static bool exists(const std::string &path) {
        return INVALID_FILE_ATTRIBUTES != GetFileAttributesA(path.c_str()) || GetLastError() != ERROR_FILE_NOT_FOUND;
    }

    static std::string getNameWithoutExtension(const std::string &path, bool supportDirectories) {
        const size_t fileNameStartIndex = getFileNameStartIndex(path, supportDirectories);
        const size_t dotIndex = path.find_last_of('.');
        if (dotIndex == std::string::npos || dotIndex < fileNameStartIndex) {
            return path.substr(fileNameStartIndex);
        }
        return path.substr(fileNameStartIndex, dotIndex - fileNameStartIndex);
    }

    static std::string getExtension(const std::string &path, bool supportDirectories) {
        const size_t fileNameStartIndex = getFileNameStartIndex(path, supportDirectories);
        const size_t dotIndex = path.find_last_of('.');
        if (dotIndex == std::string::npos || dotIndex < fileNameStartIndex) {
            return "";
        }
        return path.substr(dotIndex + 1);
    }

private:
    static size_t getFileNameStartIndex(const std::string &path, bool supportDirectories) {
        if (!supportDirectories) {
            return 0u;
        }

        const size_t backslashIndex = path.find_last_of('\\');
        const size_t frontslashIndex = path.find_last_of('/');
        if (backslashIndex == std::string::npos && frontslashIndex == std::string::npos) {
            return 0u;
        }

        return ((frontslashIndex != std::string::npos) ? frontslashIndex : backslashIndex) + 1;
    }
};
