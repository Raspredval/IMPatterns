#pragma once
static_assert(__cplusplus >= 202506, "requires C++26 minimum version");

#include <cstdio>
#include <cctype>
#include <any>
#include <span>
#include <vector>
#include <concepts>

#include "Match.hpp"
#include "FixedString.hpp"

#define IMP_DECL_CPATTERN(name) static imp::Match name (FILE* f, imp::CapturesList& g, const std::any& u)
#define IMP_MAKE_CPATTERN(name, fn) IMP_DECL_CPATTERN(name) { return (fn)(f, g, u); }

namespace imp {
    using Captures =
        std::vector<Match>;
    using CapturesList =
        std::vector<Captures>;
    using CapturesView =
        std::span<const Match>;

    template<typename Fn>
    concept Pattern =
        std::is_class_v<Fn> &&
        std::same_as<std::invoke_result_t<const Fn, FILE*, CapturesList&, const std::any&>, Match>;

    namespace __impl {
        class StreamPos {
        public:
            StreamPos(FILE* hStream, CapturesList& groups) :
                iCurPos(ftell(hStream)),
                uCaptN((!groups.empty() ? groups.back().size() : 0)) {}

            StreamPos(intptr_t iCurPos, CapturesList& groups) :
                iCurPos(iCurPos),
                uCaptN((!groups.empty() ? groups.back().size() : 0)) {}

            StreamPos(intptr_t iCurPos, size_t uCaptN) :
                iCurPos(iCurPos),
                uCaptN(uCaptN) {}

            void
            RestoreStream(FILE* hStream, CapturesList& groups) {
                fseek(hStream, this->iCurPos, SEEK_SET);
                if (!groups.empty()) {
                    while (groups.back().size() > this->uCaptN)
                        groups.back().pop_back();
                }
            }

            intptr_t
            CurrentPos() const noexcept {
                return this->iCurPos;
            }

            size_t
            CaptureCount() const noexcept {
                return this->uCaptN;
            }

        private:
            intptr_t
                iCurPos;
            size_t
                uCaptN;
        };
    }

    inline constexpr Pattern auto
    Join(const Pattern auto&... args) {
        return [tplArgs = std::make_tuple(args...)]
        (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mAcc    = Match(ftell(hFile), 0uz);
            template for (const Pattern auto& fn : tplArgs) {
                mAcc    += fn(hFile, groups, usr_val);
                if (!mAcc)
                    break;
            }

            return mAcc;
        };
    }

    inline constexpr Pattern auto
    Choice(const Pattern auto&... args) {
        return [tplArgs = std::make_tuple(args...)]
        (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
            __impl::StreamPos
                p   = {hFile, groups};
            template for (const Pattern auto& fn : tplArgs) {
                Match
                    mCur    = fn(hFile, groups, usr_val);
                if (mCur)
                    return mCur;
                else
                    p.RestoreStream(hFile, groups);
            }

            return Match(p.CurrentPos(), 0uz, false);
        };
    }

    template<FixedString strMatch>
    inline constexpr Pattern auto
    Str() {
        return [] (FILE* hFile, CapturesList&, const std::any&) -> Match {
            intptr_t
                iBegin  = ftell(hFile);
            for (size_t i = 0; i != strMatch.size(); ++i) {
                int optc    = fgetc(hFile);
                if (optc == EOF || strMatch[i] != (char)optc)
                    return Match(iBegin, i, false);
            }

            return Match(iBegin, strMatch.size());
        };
    }

    template<FixedString strSet>
    inline constexpr Pattern auto
    Set() {
        return [] (FILE* hFile, CapturesList&, const std::any&) -> Match {
            intptr_t
                iBegin  = ftell(hFile);

            int
                optc    = fgetc(hFile);
            if (optc == EOF)
                return Match(iBegin, 0uz, false);

            Match
                mCur    = Match(iBegin, 1uz);
            if (!strSet.contains((char)optc))
                mCur.ToggleGood();

            return mCur;
        };
    }

    namespace __impl {
        using CTypeProc =
            int(*)(int);

        template<CTypeProc fnCheck>
        inline constexpr Pattern auto
        CType() {
            return [] (FILE* hFile, CapturesList&, const std::any&) -> Match {
                intptr_t
                    iBegin  = ftell(hFile);

                int
                    optc    = fgetc(hFile);
                if (optc == EOF)
                    return Match(iBegin, 0uz, false);

                Match
                    mCur    = Match(iBegin, 1uz);
                if (!fnCheck(optc))
                    mCur.ToggleGood();

                return mCur;
            };
        }
    }

    inline constexpr Pattern auto
    Digit() {
        return __impl::CType<isdigit>();
    }

    inline constexpr Pattern auto
    HexDigit() {
        return __impl::CType<isxdigit>();
    }

    inline constexpr Pattern auto
    Alpha() {
        return __impl::CType<isalpha>();
    }

    inline constexpr Pattern auto
    Alnum() {
        return __impl::CType<isalnum>();
    }

    inline constexpr Pattern auto
    Space() {
        return __impl::CType<isblank>();
    }

    inline constexpr Pattern auto
    SpaceOrNewLine() {
        return __impl::CType<isspace>();
    }

    namespace __impl {
        template<bool bAny>
        inline constexpr Pattern auto
        AnyOrNone() {
            return [] (FILE* hFile, CapturesList&, const std::any&) -> Match {
                intptr_t
                    iBegin  = ftell(hFile);
                return (fgetc(hFile) == EOF)
                    ? Match(iBegin, 0uz, !bAny)
                    : Match(iBegin, 1uz, bAny);
            };
        }
    }

    inline constexpr Pattern auto
    Any() {
        return __impl::AnyOrNone<true>();
    }

    inline constexpr Pattern auto
    None() {
        return __impl::AnyOrNone<false>();
    }

    template<size_t n>
    inline constexpr Pattern auto
    UpTo(const Pattern auto& fn) {
        return [fn] (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mAcc    = Match(ftell(hFile), 0uz);
            for (size_t i = 0; i != n; ++i) {
                __impl::StreamPos
                    p       = {mAcc.End(), groups};
                Match
                    mCur    = fn(hFile, groups, usr_val);
                if (!mCur) {
                    p.RestoreStream(hFile, groups);
                    break;
                }

                mAcc    += mCur;
            }

            return mAcc;
        };
    }

    template<size_t n>
    inline Pattern auto
    AtLeast(const Pattern auto& fn) {
        return [fn] (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mAcc    = Match(ftell(hFile), 0uz);
            for (size_t i = 0; i != n; ++i) {
                mAcc    += fn(hFile, groups, usr_val);
                if (!mAcc)
                    return mAcc;
            }

            while (true) {
                __impl::StreamPos
                    p       = {mAcc.End(), groups};
                Match
                    mCur    = fn(hFile, groups, usr_val);
                if (!mCur) {
                    p.RestoreStream(hFile, groups);
                    break;
                }

                mAcc    += mCur;
            }

            return mAcc;
        };
    }

    template<size_t n>
    inline Pattern auto
    Exactly(const Pattern auto& fn) {
        return [fn] (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mAcc    = Match(ftell(hFile), 0uz);
            for (size_t i = 0; i != n; ++i) {
                mAcc    += fn(hFile, groups, usr_val);
                if (!mAcc)
                    break;
            }

            return mAcc;
        };
    }

    template<typename T>
    concept Handler =
        std::same_as<std::invoke_result_t<T, FILE*, const Match&, CapturesView, const std::any&>, void>;

    inline Pattern auto
    Handle(const Pattern auto& fn, const Handler auto& handler) {
        return [fn, handler] (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mCur    = fn(hFile, groups, usr_val);
            CapturesView
                spnCapt = (groups.empty())
                    ? CapturesView{} : CapturesView{groups.back()};
            handler(hFile, mCur, spnCapt, usr_val);
            return mCur;
        };
    }

    inline Pattern auto
    CaptGr(const Pattern auto& fn) {
        return [fn] (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
            groups.emplace_back();
            Match
                mCur    = fn(hFile, groups, usr_val);
            groups.pop_back();
            return mCur;
        };
    }

    inline Pattern auto
    CaptGr(const Pattern auto& fn, const Handler auto& handler) {
        return [fn, handler] (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
            groups.emplace_back();
            Match
                mCur    = fn(hFile, groups, usr_val);
            CapturesView
                spnCapt = (groups.empty())
                    ? CapturesView{} : CapturesView{groups.back()};
            handler(hFile, mCur, spnCapt, usr_val);
            groups.pop_back();
            return mCur;
        };
    }

    inline Pattern auto
    Capt(const Pattern auto& fn) {
        return [fn] (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mCur    = fn(hFile, groups, usr_val);
            if (mCur) {
                groups.at(groups.size() - 1)
                    .push_back(mCur);
            }

            return mCur;
        };
    }

    inline Pattern auto
    LookAhead(const Pattern auto& fn) {
        return [fn] (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
            size_t
                uCaptN  = (!groups.empty())
                            ? groups.back().size() : 0;
            Match
                mCur    = fn(
                            hFile, groups, usr_val);
            __impl::StreamPos{mCur.Begin(), uCaptN}
                .RestoreStream(hFile, groups);
            return Match(mCur.Begin(), 0uz, mCur.Good());
        };
    }

    inline Pattern auto
    Not(const Pattern auto& fn) {
        return [fn] (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mCur    = fn(hFile, groups, usr_val);
            mCur.ToggleGood();
            return mCur;
        };
    }

    using CPattern =
        Match(*)(FILE*, CapturesList&, const std::any&);

    template<__impl::CPattern fn>
    inline Pattern auto
    Fn() {
        return [] (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
            return fn(hFile, groups, usr_val);
        };
    }

    inline Match
    Eval(const Pattern auto& fn, FILE* hFile, CapturesList& groups, const std::any& usr_val = {}) {
        return fn(hFile, groups, usr_val);
    }

    inline Match
    Eval(const Pattern auto& fn, FILE* hFile, const std::any& usr_val = {}) {
        CapturesList
            groups;
        return Eval(fn, hFile, groups, usr_val);
    }
}