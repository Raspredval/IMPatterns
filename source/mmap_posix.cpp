#ifndef _WIN32
#include "MappedFile.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>


namespace imp::__impl {
    extern std::span<const char>
    mmap_posix::map_file(std::string_view strvFilename) noexcept {
        struct stat
            file_info   = {};
        int
            hFile       = open(strvFilename.data(), O_RDONLY);
        auto
            defer_file  = defer{[hFile] { if (hFile > 0) close(hFile); }};
        if (hFile < 0 || fstat(hFile, &file_info) < 0)
            return {};

        size_t
            uFileSize   = (size_t)file_info.st_size;
        void*
            lpFileView  = mmap(
                            NULL, uFileSize,
                            PROT_READ, MAP_PRIVATE,
                            hFile, 0);
        if (lpFileView == MAP_FAILED)
            return {};

        return {
            (const char*)lpFileView, uFileSize
        };
    }

    extern bool
    mmap_posix::unmap_file(std::span<const char> spnFileView) noexcept {
        if (spnFileView.data())
            return !munmap((void*)spnFileView.data(), spnFileView.size());
        else
            return false;
    }
}

#endif