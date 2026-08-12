#pragma once
static_assert(__cplusplus >= 202207L, "requires C++23 minimum version");

#include <cstdio>
#include <cctype>
#include <any>
#include <span>
#include <vector>
#include <concepts>

#include "Match.hpp"
#include "MemStream.hpp"
#include "FixedString.hpp"

#define IMP_DECL_RULE(name) imp::Match name (imp::MemStream& s, imp::CapturesList& g, const std::any& u)
#define IMP_MAKE_RULE(name, fn) IMP_DECL_RULE(name) { return (fn)(s, g, u); }

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
        std::same_as<std::invoke_result_t<const Fn, MemStream&, CapturesList&, const std::any&>, Match>;

    namespace __impl {
        inline void
        RestoreState(intptr_t iOldPos, size_t uOldCptCnt, MemStream& stream, CapturesList& groups) noexcept {
            stream.SetPos(iOldPos);
            while (groups.size() > uOldCptCnt)
                groups.pop_back();
        }
    }

    inline constexpr Pattern auto
    operator>>(const Pattern auto& lhs, const Pattern auto& rhs) {
        return [tpl = std::make_tuple(lhs, rhs)]
        (MemStream& stream, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mLhs    = std::get<0>(tpl)(stream, groups, usr_val);
            if (!mLhs)
                return mLhs;
            return mLhs + std::get<1>(tpl)(stream, groups, usr_val);
        };
    }

    inline constexpr Pattern auto
    operator|(const Pattern auto& lhs, const Pattern auto& rhs) {
        return [tpl = std::make_tuple(lhs, rhs)]
        (MemStream& stream, CapturesList& groups, const std::any& usr_val) -> Match {
            intptr_t
                iBegin  = stream.GetPos();
            size_t
                uCptCnt = groups.size();
            Match
                mLhs    = std::get<0>(tpl)(stream, groups, usr_val);
            if (mLhs)
                return mLhs;
            __impl::RestoreState(iBegin, uCptCnt, stream, groups);
            return std::get<1>(tpl)(stream, groups, usr_val);
        };
    }

    template<FixedString strMatch>
    inline constexpr Pattern auto
    Str() {
        return []
        (MemStream& stream, CapturesList&, const std::any&) -> Match {
            intptr_t
                iBegin  = stream.GetPos();
            for (size_t i = 0; i != strMatch.size(); ++i) {
                auto optc   = stream.Read();
                if (!optc || strMatch[i] != *optc)
                    return Match(iBegin, i, false);
            }

            return Match(iBegin, strMatch.size());
        };
    }

    namespace __impl {
        template<FixedString strSet, bool bSet>
        inline constexpr Pattern auto
        SetOrNegSet() {
            return []
            (MemStream& stream, CapturesList&, const std::any&) -> Match {
                intptr_t
                    iBegin  = stream.GetPos();
                auto optc   = stream.Read();
                if (!optc)
                    return Match(iBegin, 0uz, false);

                Match
                    mCur    = Match(iBegin, 1uz, bSet);
                if (!strSet.contains(*optc))
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
            return []
            (MemStream& stream, CapturesList&, const std::any&) -> Match {
                intptr_t
                    iBegin  = stream.GetPos();
                auto optc   = stream.Read();
                if (!optc)
                    return Match(iBegin, 0uz, false);

                Match
                    mCur    = Match(iBegin, 1uz);
                if (!fnCheck(*optc))
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
            return []
            (MemStream& stream, CapturesList&, const std::any&) -> Match {
                intptr_t
                    iBegin  = stream.GetPos();
                return ((bool)stream.Read())
                    ? Match(iBegin, 1uz, bAny)
                    : Match(iBegin, 0uz, !bAny);
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

    template<size_t n> requires (n > 0)
    inline constexpr Pattern auto
    UpTo(const Pattern auto& fn) {
        return [fn]
        (MemStream& stream, CapturesList& groups, const std::any& usr_val) -> Match {
            intptr_t
                iBegin  = stream.GetPos();
            Match
                mAcc    = Match(iBegin, 0uz);
            for (size_t i = 0; i != n; ++i) {
                size_t
                    uCptCnt = groups.size();
                Match
                    mCur    = fn(stream, groups, usr_val);
                if (!mCur) {
                    __impl::RestoreState(mAcc.End(), uCptCnt, stream, groups);
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
        return [fn]
        (MemStream& stream, CapturesList& groups, const std::any& usr_val) -> Match {
            intptr_t
                iBegin  = stream.GetPos();
            Match
                mAcc    = Match(iBegin, 0uz);
            for (size_t i = 0; i != n; ++i) {
                mAcc    += fn(stream, groups, usr_val);
                if (!mAcc)
                    return mAcc;
            }

            while (true) {
                size_t
                    uCptCnt = groups.size();
                Match
                    mCur    = fn(stream, groups, usr_val);
                if (!mCur) {
                    __impl::RestoreState(mAcc.End(), uCptCnt, stream, groups);
                    break;
                }

                mAcc    += mCur;
            }

            return mAcc;
        };
    }

    template<size_t n> requires (n > 0)
    inline Pattern auto
    Exactly(const Pattern auto& fn) {
        return [fn]
        (MemStream& stream, CapturesList& groups, const std::any& usr_val) -> Match {
            intptr_t
                iBegin  = stream.GetPos();
            Match
                mAcc    = Match(iBegin, 0uz);
            for (size_t i = 0; i != n; ++i) {
                mAcc    += fn(stream, groups, usr_val);
                if (!mAcc)
                    break;
            }

            return mAcc;
        };
    }

    template<typename Fn>
    concept Handler =
        std::is_class_v<Fn> &&
        std::same_as<std::invoke_result_t<Fn, MemStream&, const Match&, CapturesView, const std::any&>, Match>;

    inline Pattern auto
    operator/(const Pattern auto& fn, const Handler auto& handler) {
        return [tpl = std::make_tuple(fn, handler)]
        (MemStream& stream, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mCur    = std::get<0>(tpl)(stream, groups, usr_val);
            CapturesView
                spnCapt = (groups.empty())
                    ? CapturesView{} : CapturesView{groups.back()};
            mCur        = std::get<1>(tpl)(stream, mCur, spnCapt, usr_val);
            return mCur;
        };
    }

    inline Pattern auto
    CaptGr(const Pattern auto& fn) {
        return [fn]
        (MemStream& stream, CapturesList& groups, const std::any& usr_val) -> Match {
            groups.emplace_back();
            Match
                mCur    = fn(stream, groups, usr_val);
            groups.pop_back();
            return mCur;
        };
    }

    inline Pattern auto
    CaptGr(const Pattern auto& fn, const Handler auto& handler) {
        return [tpl = std::make_tuple(fn, handler)]
        (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
            groups.emplace_back();
            Match
                mCur    = std::get<0>(tpl)(hFile, groups, usr_val);
            CapturesView
                spnCapt = (groups.empty())
                    ? CapturesView{} : CapturesView{groups.back()};
            mCur        = std::get<1>(tpl)(hFile, mCur, spnCapt, usr_val);
            groups.pop_back();
            return mCur;
        };
    }

    inline Pattern auto
    Capt(const Pattern auto& fn) {
        return [fn]
        (FILE* hFile, CapturesList& groups, const std::any& usr_val) -> Match {
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
        return [fn]
        (MemStream& stream, CapturesList& groups, const std::any& usr_val) -> Match {
            intptr_t
                iBegin  = stream.GetPos();
            size_t
                uCptCnt = (!groups.empty())
                            ? groups.back().size() : 0;
            Match
                mCur    = fn(stream, groups, usr_val);
            __impl::RestoreState(iBegin, uCptCnt, stream, groups);
            return Match(iBegin, 0uz, mCur.Good());
        };
    }

    inline Pattern auto
    Not(const Pattern auto& fn) {
        return [fn]
        (MemStream& stream, CapturesList& groups, const std::any& usr_val) -> Match {
            Match
                mCur    = fn(stream, groups, usr_val);
            mCur.ToggleGood();
            return mCur;
        };
    }

    using CRule =
        Match(*)(MemStream&, CapturesList&, const std::any&);

    using CHandler =
        Match(*)(MemStream&, const Match&, CapturesView, const std::any&);

    template<CRule fn>
    inline Pattern auto
    Fn() {
        return []
        (MemStream& s, CapturesList& g, const std::any& u) -> Match {
            return fn(s, g, u);
        };
    }

    template<CHandler fn>
    inline Handler auto
    Fn() {
        return []
        (MemStream& s, const Match& m, CapturesView c, const std::any& u) -> Match {
            return fn(s, m, c, u);
        };
    }

    inline Match
    Eval(const Pattern auto& fn, MemStream& stream, CapturesList& groups, const std::any& usr_val = {}) {
        return fn(stream, groups, usr_val);
    }

    inline Match
    Eval(const Pattern auto& fn, MemStream& stream, const std::any& usr_val = {}) {
        CapturesList
            groups;
        return Eval(fn, stream, groups, usr_val);
    }
}