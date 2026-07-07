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

        template<size_t temp_buff_size = BUFSIZ>
        void
        ExportTo(FILE* hFrom, FILE* hTo) const {
            char
                lpcBuffer[temp_buff_size];
            intptr_t
                iCur    = ftell(hFrom);
            fseek(hFrom, this->iBegin, SEEK_SET);

            size_t
                uTotal  = this->uLength;
            while (uTotal != 0) {
                uTotal -=   fwrite(
                                lpcBuffer, 1,
                                fread(
                                    lpcBuffer, 1,
                                    std::min(uTotal, temp_buff_size),
                                    hFrom),
                                hTo);
                if (ferror(hFrom) || ferror(hTo) || feof(hFrom))
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
            strResult.resize(this->uLength);

            fseek(hFrom, this->iBegin, SEEK_SET);
            strResult.resize(
                fread(
                    strResult.data(), 1,
                    this->uLength, hFrom));
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