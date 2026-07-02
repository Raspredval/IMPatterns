#pragma once
static_assert(__cplusplus >= 202506, "requires C++26 minimum version");

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <string>

namespace imp {
    namespace __impl {
        template<size_t bitn, std::integral I>
        inline constexpr I
        mask_bit(I num) {
            return num & ~(I{1} << bitn);
        }
    }

    class Match {
    public:
        Match(intptr_t iBegin, size_t uLength, bool bGood = true) :
            iBegin(iBegin),
            uLength(uLength & ~(1uz << (sizeof(size_t) * 8 - 1))),
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

        void
        ExportTo(FILE* hFrom, FILE* hTo) const {
            intptr_t
                iCur    = ftell(hFrom);
            fseek(hFrom, this->iBegin, SEEK_SET);
            for (size_t i = 0; i != this->Length(); ++i) {
                int
                    optc    = fgetc(hFrom);
                if (optc == EOF || fputc(optc, hTo) == EOF)
                    break;
            }

            fseek(hFrom, iCur, SEEK_SET);
        }

        std::string
        GetString(FILE* hFrom) const {
            intptr_t
                iCur    = ftell(hFrom);
            std::string
                strResult;
            strResult.reserve(this->uLength);
            fseek(hFrom, this->iBegin, SEEK_SET);
            for (size_t i = 0; i != this->Length(); ++i) {
                int
                    optc    = fgetc(hFrom);
                if (optc == EOF)
                    break;
                strResult.push_back((char)optc);
            }

            fseek(hFrom, iCur, SEEK_SET);
            return strResult;
        }

    private:
        intptr_t
            iBegin;
        uintptr_t
            uLength : sizeof(uintptr_t) * 8 - 1,
            bGood   : 1;
    };
}