#ifndef _WIN32
#include "MappedFile.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>


namespace imp::__impl {
    extern std::span<const char>
    mmap_posix::map_file(std::string_view strvFilename) noexcept {
        void*
            lpFileView  = nullptr;
        size_t
            uFileSize   = 0;
        struct stat
            file_info   = {};
        std::span<const char>
            spnResult   = {};
        int
            hFile       = open(strvFilename.data(), O_RDONLY);
        if (hFile < 0)
            goto finally;

        if (fstat(hFile, &file_info) < 0)
            goto finally;
        uFileSize       = (size_t)file_info.st_size;

        lpFileView      = mmap(
                            NULL, uFileSize,
                            PROT_READ, MAP_PRIVATE,
                            hFile, 0);
        if (lpFileView == MAP_FAILED)
            goto finally;

        spnResult       = {
            std::start_lifetime_as<const char>(lpFileView), uFileSize
        };

    finally:
        if (hFile > 0)
            close(hFile);

        return spnResult;
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