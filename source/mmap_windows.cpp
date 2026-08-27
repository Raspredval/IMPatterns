#ifdef _WIN32
#include "MappedFile.hpp"
#include <windows.h>

namespace imp::__impl {
    static std::span<const char>
    map_file_handle(HANDLE hFile) noexcept;

    std::span<const char>
    mmap_windows::map_file(std::string_view strvFilename) noexcept {
        HANDLE
            hFile   = CreateFileA(
                            strvFilename.data(),
                            GENERIC_READ,
                            0, nullptr,
                            OPEN_EXISTING,
                            0, nullptr);
        return map_file_handle(hFile);
    }

    std::span<const char>
    mmap_windows::map_file(std::wstring_view strvFilename) noexcept {
        HANDLE
            hFile   = CreateFileW(
                            strvFilename.data(),
                            GENERIC_READ,
                            0, nullptr,
                            OPEN_EXISTING,
                            0, nullptr);
        return map_file_handle(hFile);
    }

    bool
    mmap_windows::unmap_file(std::span<const char> spnFileView) noexcept {
        if (spnFileView.data())
            return UnmapViewOfFile((LPCVOID)spnFileView.data());
        else
            return false;
    }

    static std::span<const char>
    map_file_handle(HANDLE hFile) noexcept {
        HANDLE
            hFileMap    = nullptr;
        void*
            lpFileView  = nullptr;
        UINT64
            uFileSize   = 0;
        BY_HANDLE_FILE_INFORMATION
            file_info   = {};
        std::span<const char>
            spnResult   = {};

        if (hFile == INVALID_HANDLE_VALUE)
            goto finally;

        if (!GetFileInformationByHandle(hFile, &file_info))
            goto finally;
        uFileSize   = (UINT64)file_info.nFileSizeLow |
                        (UINT64)file_info.nFileSizeHigh << 32;

        hFileMap    = CreateFileMappingA(
                        hFile, nullptr,
                        PAGE_READONLY,
                        (DWORD)(uFileSize >> 32),
                        (DWORD)(uFileSize),
                        nullptr);
        if (hFileMap == NULL)
            goto finally;

        lpFileView  = MapViewOfFile(
                        hFileMap,
                        FILE_MAP_READ,
                        0, 0,
                        (SIZE_T)uFileSize);
        if (lpFileView == NULL)
            goto finally;

        spnResult = {
            std::start_lifetime_as<const char>(lpFileView),
            uFileSize
        };

    finally:
        if (hFileMap != NULL)
            CloseHandle(hFileMap);
        if (hFile != INVALID_HANDLE_VALUE)
            CloseHandle(hFile);

        return spnResult;
    }
}

#endif