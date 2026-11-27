#pragma once
static_assert(__cplusplus >= 202302L, "requires C++23 minimum version");

#include <span>
#include <string_view>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace imp {
    namespace ipc = boost::interprocess;
    class MappedFile {
    public:
        MappedFile(std::string_view strvFilename) :
            mfile(strvFilename.data(), ipc::read_only)
        {
            this->initMappedRegion();
        }

        #ifdef _WIN32
        MappedFile(std::wstring_view strvFilename) :
            mfile(strvFilename.data(), ipc::read_only)
        {
            this->initMappedRegion();
        }
        #endif

        std::span<const char>
        GetView() const noexcept {
            return {
                (const char*)this->mview.get_address(),
                this->mview.get_size()
            };
        }

        bool
        Bad() const noexcept {
            return this->GetView().empty();
        }

        operator bool() const noexcept {
            return !this->Bad();
        }

    private:
        void
        initMappedRegion() {
            ipc::offset_t
                iFileSize   = -1;
            if (this->mfile.get_size(iFileSize)) {
                this->mview = ipc::mapped_region(
                                this->mfile, ipc::read_only,
                                0, (size_t)iFileSize);
            }
        }

        ipc::file_mapping
            mfile;
        ipc::mapped_region
            mview;
    };
}