#pragma once
static_assert(__cplusplus >= 202002L, "requires C++23 minimum version");

#include <span>
#include <cstdint>
#include <optional>
#include <algorithm>

namespace imp {
    class MemStream {
    public:
        MemStream(std::span<const char> spnData) :
            spnData(spnData), iPos(0)
        {
            if (spnData.data() == nullptr)
                this->iPos  = -1;
        }

        inline bool
        Bad() const noexcept {
            return this->iPos < 0;
        }

        inline
        operator bool() const noexcept {
            return !this->Bad();
        }

        inline std::optional<char>
        Read() noexcept {
            if (this->iPos < 0 || (size_t)this->iPos == this->spnData.size())
                return std::nullopt;
            return this->spnData[(size_t)this->iPos++];
        }

        inline void
        SetPos(intptr_t iNewPos) noexcept {
            this->iPos  = std::clamp<intptr_t>(iNewPos,
                            0, (intptr_t)this->StreamData().size());
        }

        inline intptr_t
        GetPos() const noexcept {
            return this->iPos;
        }

        inline std::span<const char>
        StreamData() const noexcept {
            return this->spnData;
        }

    private:
        std::span<const char>
            spnData;
        intptr_t
            iPos;
    };
}