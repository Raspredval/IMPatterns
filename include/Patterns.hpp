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

#define IMP_DECL_PATTERN(name) imp::Match name (FILE* f, intptr_t i, imp::CapturesList& g, const std::any& u)
#define IMP_MAKE_PATTERN(name, fn) IMP_DECL_PATTERN(name) { return (fn)(f, i, g, u); }

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
        std::same_as<std::invoke_result_t<const Fn, FILE*, intptr_t, CapturesList&, const std::any&>, Match>;

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
        (FILE* hFile, intptr_t iBegin, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mAcc    = Match(iBegin, 0uz);
            template for (const Pattern auto& fn : tplArgs) {
                mAcc    += fn(hFile, mAcc.End(), groups, usr_val);
                if (!mAcc)
                    break;
            }

            return mAcc;
        };
    }

    inline constexpr Pattern auto
    Choice(const Pattern auto&... args) {
        return [tplArgs = std::make_tuple(args...)]
        (FILE* hFile, intptr_t iBegin, CapturesList& groups, const std::any& usr_val) -> Match {
            __impl::StreamPos
                p   = {iBegin, groups};
            template for (const Pattern auto& fn : tplArgs) {
                Match
                    mCur    = fn(hFile, p.CurrentPos(), groups, usr_val);
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
        return [] (FILE* hFile, intptr_t iBegin, CapturesList&, const std::any&) -> Match {
            for (size_t i = 0; i != strMatch.size(); ++i) {
                int optc    = fgetc(hFile);
                if (optc == EOF || strMatch[i] != (char)optc)
                    return Match(iBegin, i, false);
            }

            return Match(iBegin, strMatch.size());
        };
    }

    namespace __impl {
        template<FixedString strSet, bool bSet>
        inline constexpr Pattern auto
        SetOrNegSet() {
            return [] (FILE* hFile, intptr_t iBegin, CapturesList&, const std::any&) -> Match {
                int
                    optc    = fgetc(hFile);
                if (optc == EOF)
                    return Match(iBegin, 0uz, false);

                Match
                    mCur    = Match(iBegin, 1uz, bSet);
                if (!strSet.contains((char)optc))
                    mCur.ToggleGood();

                return mCur;
            };
        }
    }

    template<FixedString strSet>
    inline constexpr Pattern auto
    Set() {
        return __impl::SetOrNegSet<strSet, true>();
    }

    template<FixedString strSet>
    inline constexpr Pattern auto
    NegSet() {
        return __impl::SetOrNegSet<strSet, false>();
    }

    namespace __impl {
        using CTypeProc =
            int(*)(int);

        template<CTypeProc fnCheck>
        inline constexpr Pattern auto
        CType() {
            return [] (FILE* hFile, intptr_t iBegin, CapturesList&, const std::any&) -> Match {
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
            return [] (FILE* hFile, intptr_t iBegin, CapturesList&, const std::any&) -> Match {
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
        return [fn] (FILE* hFile, intptr_t iBegin, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mAcc    = Match(iBegin, 0uz);
            for (size_t i = 0; i != n; ++i) {
                __impl::StreamPos
                    p       = {mAcc.End(), groups};
                Match
                    mCur    = fn(hFile, p.CurrentPos(), groups, usr_val);
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
        return [fn] (FILE* hFile, intptr_t iBegin, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mAcc    = Match(iBegin, 0uz);
            for (size_t i = 0; i != n; ++i) {
                mAcc    += fn(hFile, mAcc.End(), groups, usr_val);
                if (!mAcc)
                    return mAcc;
            }

            while (true) {
                __impl::StreamPos
                    p       = {mAcc.End(), groups};
                Match
                    mCur    = fn(hFile, p.CurrentPos(), groups, usr_val);
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
        return [fn] (FILE* hFile, intptr_t iBegin, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mAcc    = Match(iBegin, 0uz);
            for (size_t i = 0; i != n; ++i) {
                mAcc    += fn(hFile, mAcc.End(), groups, usr_val);
                if (!mAcc)
                    break;
            }

            return mAcc;
        };
    }

    template<typename Fn>
    concept Handler =
        std::is_class_v<Fn> &&
        std::same_as<std::invoke_result_t<Fn, FILE*, const Match&, CapturesView, const std::any&>, Match>;

    inline Pattern auto
    Handle(const Pattern auto& fn, const Handler auto& handler) {
        return [fn, handler] (FILE* hFile, intptr_t iBegin, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mCur    = fn(hFile, iBegin, groups, usr_val);
            CapturesView
                spnCapt = (groups.empty())
                    ? CapturesView{} : CapturesView{groups.back()};
            mCur        = handler(hFile, mCur, spnCapt, usr_val);
            return mCur;
        };
    }

    inline Pattern auto
    CaptGr(const Pattern auto& fn) {
        return [fn] (FILE* hFile, intptr_t iBegin, CapturesList& groups, const std::any& usr_val) -> Match {
            groups.emplace_back();
            Match
                mCur    = fn(hFile, iBegin, groups, usr_val);
            groups.pop_back();
            return mCur;
        };
    }

    inline Pattern auto
    CaptGr(const Pattern auto& fn, const Handler auto& handler) {
        return [fn, handler] (FILE* hFile, intptr_t iBegin, CapturesList& groups, const std::any& usr_val) -> Match {
            groups.emplace_back();
            Match
                mCur    = fn(hFile, iBegin, groups, usr_val);
            CapturesView
                spnCapt = (groups.empty())
                    ? CapturesView{} : CapturesView{groups.back()};
            mCur        = handler(hFile, mCur, spnCapt, usr_val);
            groups.pop_back();
            return mCur;
        };
    }

    inline Pattern auto
    Capt(const Pattern auto& fn) {
        return [fn] (FILE* hFile, intptr_t iBegin, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mCur    = fn(hFile, iBegin, groups, usr_val);
            if (mCur) {
                groups.at(groups.size() - 1)
                    .push_back(mCur);
            }

            return mCur;
        };
    }

    inline Pattern auto
    LookAhead(const Pattern auto& fn) {
        return [fn] (FILE* hFile, intptr_t iBegin, CapturesList& groups, const std::any& usr_val) -> Match {
            size_t
                uCaptN  = (!groups.empty())
                            ? groups.back().size() : 0;
            Match
                mCur    = fn(
                            hFile, groups, usr_val);
            __impl::StreamPos{iBegin, uCaptN}
                .RestoreStream(hFile, groups);
            return Match(iBegin, 0uz, mCur.Good());
        };
    }

    inline Pattern auto
    Not(const Pattern auto& fn) {
        return [fn] (FILE* hFile, intptr_t iBegin, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mCur    = fn(hFile, iBegin, groups, usr_val);
            mCur.ToggleGood();
            return mCur;
        };
    }

    using UserPattern =
        Match(*)(FILE*, intptr_t, CapturesList&, const std::any&);

    using UserHandler =
        Match(*)(FILE*, const Match&, CapturesView, const std::any&);

    template<UserPattern fn>
    inline Pattern auto
    Fn() {
        return [] (FILE* f, intptr_t i, CapturesList& g, const std::any& u) -> Match {
            return fn(f, i, g, u);
        };
    }

    template<UserHandler fn>
    inline Handler auto
    Fn() {
        return [] (FILE* f, const Match& m, CapturesView c, const std::any& u) -> Match {
            return fn(f, m, c, u);
        };
    }

    inline Match
    Eval(const Pattern auto& fn, FILE* hFile, CapturesList& groups, const std::any& usr_val = {}) {
        return fn(hFile, ftell(hFile), groups, usr_val);
    }

    inline Match
    Eval(const Pattern auto& fn, FILE* hFile, const std::any& usr_val = {}) {
        CapturesList
            groups;
        return Eval(fn, hFile, groups, usr_val);
    }
}