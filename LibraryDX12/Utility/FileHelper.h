#pragma once

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/windows.h>
#include <string>

struct FileHelper : DXD::NonCopyableAndMovable {
    FileHelper() = delete;

    static bool exists(const std::string &path) {
        return INVALID_FILE_ATTRIBUTES != GetFileAttributesA(path.c_str()) || GetLastError() != ERROR_FILE_NOT_FOUND;
    }

    static bool exists(const std::wstring &path) {
        return INVALID_FILE_ATTRIBUTES != GetFileAttributesW(path.c_str()) || GetLastError() != ERROR_FILE_NOT_FOUND;
    }

    template <typename CharT>
    static std::basic_string<CharT> getNameWithoutExtension(const std::basic_string<CharT> &path, bool supportDirectories) {
        const size_t fileNameStartIndex = getFileNameStartIndex(path, supportDirectories);
        const size_t dotIndex = path.find_last_of('.');
        if (dotIndex == std::basic_string<CharT>::npos || dotIndex < fileNameStartIndex) {
            return path.substr(fileNameStartIndex);
        }
        return path.substr(fileNameStartIndex, dotIndex - fileNameStartIndex);
    }

    template <typename CharT>
    static std::basic_string<CharT> getExtension(const std::basic_string<CharT> &path, bool supportDirectories) {
        const size_t fileNameStartIndex = getFileNameStartIndex(path, supportDirectories);
        const size_t dotIndex = path.find_last_of('.');
        if (dotIndex == std::basic_string<CharT>::npos || dotIndex < fileNameStartIndex) {
            return std::basic_string<CharT>{};
        }
        return path.substr(dotIndex + 1);
    }

private:
    template <typename CharT>
    static size_t getFileNameStartIndex(const std::basic_string<CharT> &path, bool supportDirectories) {
        if (!supportDirectories) {
            return 0u;
        }

        const size_t backslashIndex = path.find_last_of('\\');
        const size_t frontslashIndex = path.find_last_of('/');
        if (backslashIndex == std::basic_string<CharT>::npos && frontslashIndex == std::basic_string<CharT>::npos) {
            return 0u;
        }

        return ((frontslashIndex != std::basic_string<CharT>::npos) ? frontslashIndex : backslashIndex) + 1;
    }
};
