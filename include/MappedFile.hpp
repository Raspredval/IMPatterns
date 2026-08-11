#pragma once
#include <span>
#include <cstdint>
#include <string_view>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

namespace imp {
    namespace __impl {
        #ifdef _WIN32
        // !!! NOT TESTED, NOT SURE IF IT EVEN COMPILES !!!
        // TODO: test in actual windows environment
        struct mmap_windows {
            static inline std::span<const char>
            map_file(std::string_view strvFilename) noexcept {
                void*
                    lpFileView  = nullptr;
                uint64_t
                    uFileSize   = 0;
                BY_HANDLE_FILE_INFORMATION
                    file_info   = {};
                std::span<const char>
                    spnResult   = {};
                HANDLE
                    hFile       = INVALID_HANDLE_VALUE;
                HANDLE
                    hFileMap    = NULL;

                hFile           = CreateFileA(
                                    strvFilename.data(),
                                    GENERIC_READ,
                                    0, nullptr,
                                    OPEN_EXISTING, 0);
                if (hFile == INVALID_HANDLE_VALUE)
                    goto finally;

                if (!GetFileInformationByHandle(hFile, &file_info))
                    goto finally;
                uFileSize   = (uint64_t)file_info.nFileSizeLow |
                                (uint64_t)file_info.nFileSizeHigh << 32;

                hFileMap    = CreateFileMappingA(
                                hFile, nullptr,
                                PAGE_READONLY,
                                (uint32_t)(uFileSize >> 32),
                                (uint32_t)uFileSize,
                                nullptr);
                if (hFileMap == NULL)
                    goto finally;

                lpFileView  = MapViewOfFile(
                                hFileMap,
                                FILE_MAP_READ,
                                0, 0,
                                uFileSize);
                if (lpFileView == NULL)
                    goto finally;

                spnFileInfo = {
                    (const char*)lpFileView,
                    file_info.
                }

            finally:
                if (hFile != INVALID_HANDLE_VALUE)
                    CloseHandle(hFile);
                if (hFileMap != NULL)
                    CloseHandle(hFileMap);

                return spnResult;
            }

            static inline bool
            unmap_file(std::span<const char> spnFileView) noexcept {
                return UnmapViewOfFile((LPCVOID)spnFileView.data());
            }
        };
        #else
        struct mmap_posix {
            static inline std::span<const char>
            map_file(std::string_view strvFilename) noexcept {
                void*
                    lpFileView  = nullptr;
                uint64_t
                    uFileSize   = 0;
                struct stat
                    file_info   = {};
                std::span<const char>
                    spnResult   = {};
                int
                    hFile       = -1;

                hFile           = open(strvFilename.data(), O_RDONLY);
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
                    (const char*)lpFileView, uFileSize
                };

            finally:
                if (hFile > 0)
                    close(hFile);

                return spnResult;
            }

            static inline bool
            unmap_file(std::span<const char> spnFileView) noexcept {
                return !munmap((void*)spnFileView.data(), spnFileView.size());
            }
        };
        #endif

        template<typename impl>
        class MappedFile {
        public:
            inline
            MappedFile(std::string_view strvFilename) noexcept {
                this->spnData   = impl::map_file(strvFilename);
            }

            inline
            ~MappedFile() noexcept {
                impl::unmap_file(this->spnData);
            }

            MappedFile(const MappedFile&) = delete;
            MappedFile&
            operator=(const MappedFile&) = delete;

            inline
            MappedFile(MappedFile&& obj) noexcept {
                this->spnData   = obj.spnData;
                obj.spnData     = {};
            }

            inline MappedFile&
            operator=(MappedFile&& obj) noexcept {
                MappedFile
                    temp    = std::move(obj);
                std::swap(temp.spnData, this->spnData);
                return *this;
            }

            inline std::span<const char>
            Range() const noexcept {
                return this->spnData;
            }

            inline bool
            Bad() const noexcept {
                return this->spnData.data() == nullptr;
            }

            inline
            operator bool() const noexcept {
                return !this->Bad();
            }

        private:
            std::span<const char>
                spnData;
        };
    }

    #ifdef _WIN32
    using MappedFile = __impl::MappedFile<__impl::mmap_windows>;
    #else
    using MappedFile = __impl::MappedFile<__impl::mmap_posix>;
    #endif
}