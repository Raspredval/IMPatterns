#pragma once
#include <span>
#include <string_view>

namespace imp {
    namespace __impl {
        #ifdef _WIN32
        struct mmap_windows {
            static std::span<const char>
            map_file(std::string_view strvFilename) noexcept;

            static std::span<const char>
            map_file(std::wstring_view strvFilename) noexcept;

            static bool
            unmap_file(std::span<const char> spnFileView) noexcept;
        };
        #else
        struct mmap_posix {
            static std::span<const char>
            map_file(std::string_view strvFilename) noexcept;

            static bool
            unmap_file(std::span<const char> spnFileView) noexcept;
        };
        #endif

        template<typename impl>
        class MappedFile {
        public:
            inline
            MappedFile(std::string_view strvFilename) noexcept {
                this->spnData   = impl::map_file(strvFilename);
            }

            #ifdef _WIN32
            inline
            MappedFile(std::wstring_view strvFilename) noexcept {
                this->spnData   = impl::map_file(strvFilename);
            }
            #endif

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
            GetView() const noexcept {
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