#include <span>
#include <string_view>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

namespace imp {
    class MappedFile {
    public:
        inline
        MappedFile(std::string_view strvFilename) noexcept {
            struct stat
                file_info;
            void*
                lpMapped;

            int
                fdMapped    = open(strvFilename.data(), O_RDONLY);
            if (fdMapped < 0 || fstat(fdMapped, &file_info) < 0)
                goto finally;

            lpMapped        = mmap(
                                NULL, (size_t)file_info.st_size,
                                PROT_READ, MAP_PRIVATE,
                                fdMapped, 0);
            if (lpMapped == MAP_FAILED)
                goto finally;

            this->spnData   = {
                (const char*)lpMapped,
                (size_t)file_info.st_size
            };

        finally:
            if (fdMapped > 0)
                close(fdMapped);
        }

        inline
        ~MappedFile() noexcept {
            if (this->spnData.data()) {
                munmap(
                    (void*)this->spnData.data(),
                    this->spnData.size());
            }
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