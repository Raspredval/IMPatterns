#pragma once
static_assert(__cplusplus >= 202207L, "requires C++23 minimum version");

#include <cmath>
#include <cstdint>
#include <string_view>

#include "MemStream.hpp"


namespace imp {
    class Match {
    public:
        Match(intptr_t iBegin, size_t uLength, bool bGood = true) :
            iBegin(iBegin),
            uLength((uLength << 1) >> 1),
            bGood(bGood) {}

        Match(intptr_t iBegin, intptr_t iEnd, bool bGood = true) :
            Match(iBegin, (uintptr_t)std::abs(iEnd - iBegin), bGood) {}

        bool
        Good() const noexcept {
            return (bool)this->bGood;
        }

        size_t
        Length() const noexcept {
            return this->uLength;
        }

        intptr_t
        Begin() const noexcept {
            return this->iBegin;
        }

        intptr_t
        End() const noexcept {
            return this->iBegin + (intptr_t)this->uLength;
        }

        bool
        Empty() const noexcept {
            return this->Begin() == this->End();
        }

        void
        ToggleGood() noexcept {
            this->bGood = !this->bGood;
        }

        friend Match
        operator+(const Match& mLhs, const Match& mRhs) noexcept {
            return { mLhs.Begin(), mRhs.End(), mLhs.Good() && mRhs.Good() };
        }

        Match&
        operator+=(const Match& m) noexcept {
            return *this = *this + m;
        }

        operator bool() const noexcept {
            return this->Good();
        }

        std::string_view
        GetStringView(MemStream& stream) const noexcept {
            std::span<const char>
                spnData = stream.StreamData();
            return {
                spnData.data() + this->iBegin,
                this->uLength
            };
        }

    private:
        intptr_t
            iBegin;
        uintptr_t
            uLength : sizeof(uintptr_t) * 8 - 1,
            bGood   : 1;
    };
}