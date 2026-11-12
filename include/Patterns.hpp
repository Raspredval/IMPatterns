#pragma once
static_assert(__cplusplus >= 202302L, "requires C++23 minimum version");

#include <cctype>
#include <concepts>

#include "Match.hpp"
#include "Vector.hpp"
#include "CharSet.hpp"
#include "UserData.hpp"
#include "MemStream.hpp"
#include "Dictionary.hpp"
#include "FixedString.hpp"

#define IMP_DECL_RULE(name) imp::Match name (imp::MemStream& s, imp::Captures& c, imp::UserData d)
#define IMP_MAKE_RULE(name, fn) IMP_DECL_RULE(name) { return (fn)(s, c, d); }

namespace imp {
    struct Capture {
        Match
            match;
        UserData
            tag;
        Vector<Capture>
            children;
    };

    using Captures =
        Vector<Capture>;
    using CaptureView =
        std::span<const Capture>;

    template<typename Fn>
    concept Pattern =
        std::is_class_v<Fn> &&
        std::same_as<std::invoke_result_t<const Fn, MemStream&, Captures&, UserData>, Match>;

    namespace __impl {
        inline void
        RestoreState(intptr_t iOldPos, size_t uCptCnt, MemStream& stream, Captures& captures) noexcept {
            stream.SetPos(iOldPos);
            while (captures.size() > uCptCnt)
                captures.pop_back();
        }
    }

    inline constexpr Pattern auto
    operator>>(const Pattern auto& lhs, const Pattern auto& rhs) {
        return [tpl = std::make_tuple(lhs, rhs)]
        (MemStream& stream, Captures& captures, UserData usr_val) -> Match {
            Match
                mLhs    = std::get<0>(tpl)(stream, captures, usr_val);
            if (!mLhs)
                return mLhs;
            return mLhs + std::get<1>(tpl)(stream, captures, usr_val);
        };
    }

    inline constexpr Pattern auto
    operator|(const Pattern auto& lhs, const Pattern auto& rhs) {
        return [tpl = std::make_tuple(lhs, rhs)]
        (MemStream& stream, Captures& captures, UserData usr_val) -> Match {
            intptr_t
                iBegin  = stream.GetPos();
            size_t
                uCptCnt = captures.size();
            Match
                mLhs    = std::get<0>(tpl)(stream, captures, usr_val);
            if (mLhs)
                return mLhs;
            __impl::RestoreState(iBegin, uCptCnt, stream, captures);
            return std::get<1>(tpl)(stream, captures, usr_val);
        };
    }

    template<FixedString strMatch>
    inline constexpr Pattern auto
    Str() {
        return []
        (MemStream& stream, Captures&, UserData) -> Match {
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

    template<CharSet chset>
    inline constexpr Pattern auto
    Set() {
        return []
        (MemStream& stream, Captures&, UserData) -> Match {
            intptr_t
                iBegin  = stream.GetPos();
            auto optc   = stream.Read();
            if (!optc)
                return Match(iBegin, 0uz, false);

            Match
                mCur    = Match(iBegin, 1uz, true);
            if (!chset.contains(*optc))
                mCur.Toggle();

            return mCur;
        };
    }

    template<CharSet chset>
    inline constexpr Pattern auto
    NegSet() {
        return []
        (MemStream& stream, Captures&, UserData) -> Match {
            intptr_t
                iBegin  = stream.GetPos();
            auto optc   = stream.Read();
            if (!optc)
                return Match(iBegin, 0uz, false);

            Match
                mCur    = Match(iBegin, 1uz, false);
            if (!chset.contains(*optc))
                mCur.Toggle();

            return mCur;
        };
    }

    namespace __impl {
        using CTypeProc =
            int(*)(int);

        template<CTypeProc fnCheck>
        inline constexpr Pattern auto
        CType() {
            return []
            (MemStream& stream, Captures&, UserData) -> Match {
                intptr_t
                    iBegin  = stream.GetPos();
                auto optc   = stream.Read();
                if (!optc)
                    return Match(iBegin, 0uz, false);

                Match
                    mCur    = Match(iBegin, 1uz);
                if (!fnCheck(*optc))
                    mCur.Toggle();

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

    inline constexpr Pattern auto
    Uppercase() {
        return __impl::CType<isupper>();
    }

    inline constexpr Pattern auto
    Lowercase() {
        return __impl::CType<islower>();
    }

    namespace __impl {
        template<bool bAny>
        inline constexpr Pattern auto
        AnyOrNone() {
            return []
            (MemStream& stream, Captures&, UserData) -> Match {
                intptr_t
                    iBegin  = stream.GetPos();
                return ((bool)stream.Read())
                    ? Match(iBegin, 1uz, bAny)
                    : Match(iBegin, 0uz, !bAny);
            };
        }
    }

    inline constexpr Pattern auto
    AnyChar() {
        return __impl::AnyOrNone<true>();
    }

    inline constexpr Pattern auto
    NoChars() {
        return __impl::AnyOrNone<false>();
    }

    template<size_t n> requires (n > 0)
    inline constexpr Pattern auto
    UpTo(const Pattern auto& fn) {
        return [fn]
        (MemStream& stream, Captures& captures, UserData usr_val) -> Match {
            intptr_t
                iBegin  = stream.GetPos();
            Match
                mAcc    = Match(iBegin, 0uz);
            for (size_t i = 0; i != n; ++i) {
                size_t
                    uCptCnt = captures.size();
                Match
                    mCur    = fn(stream, captures, usr_val);
                if (!mCur) {
                    __impl::RestoreState(mAcc.End(), uCptCnt, stream, captures);
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
        (MemStream& stream, Captures& captures, UserData usr_val) -> Match {
            intptr_t
                iBegin  = stream.GetPos();
            Match
                mAcc    = Match(iBegin, 0uz);
            for (size_t i = 0; i != n; ++i) {
                mAcc    += fn(stream, captures, usr_val);
                if (!mAcc)
                    return mAcc;
            }

            while (true) {
                size_t
                    uCptCnt = captures.size();
                Match
                    mCur    = fn(stream, captures, usr_val);
                if (!mCur) {
                    __impl::RestoreState(mAcc.End(), uCptCnt, stream, captures);
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
        (MemStream& stream, Captures& captures, UserData usr_val) -> Match {
            intptr_t
                iBegin  = stream.GetPos();
            Match
                mAcc    = Match(iBegin, 0uz);
            for (size_t i = 0; i != n; ++i) {
                mAcc    += fn(stream, captures, usr_val);
                if (!mAcc)
                    break;
            }

            return mAcc;
        };
    }

    inline Pattern auto
    Opt(const Pattern auto& fn) {
        return UpTo<1>(fn);
    }

    inline Pattern auto
    Any(const Pattern auto& fn) {
        return AtLeast<0>(fn);
    }

    inline Pattern auto
    Some(const Pattern auto& fn) {
        return AtLeast<1>(fn);
    }

    template<typename Fn>
    concept Handler =
        std::is_class_v<Fn> &&
        std::same_as<std::invoke_result_t<const Fn, MemStream&, Match, CaptureView, UserData>, Match>;

    inline Pattern auto
    operator/=(const Pattern auto& fn, const Handler auto& handler) {
        return [tpl = std::make_tuple(fn, handler)]
        (MemStream& stream, Captures& captures, UserData usr_val) -> Match {
            Match
                mCur    = std::get<0>(tpl)(stream, captures, usr_val);

            mCur        = std::get<1>(tpl)(stream, mCur, captures, usr_val);
            return mCur;
        };
    }

    template<UserData capt_tag = {}>
    inline Pattern auto
    CaptGr(const Pattern auto& fn) {
        return [fn]
        (MemStream& stream, Captures& captures, UserData usr_val) -> Match {
            Captures
                inner_captures;
            Match
                mCur    = fn(stream, inner_captures, usr_val);
            if (mCur) {
                captures.emplace_back(
                    mCur, capt_tag,
                    std::move(inner_captures));
            }

            return mCur;
        };
    }

    template<UserData capt_tag = {}>
    inline Pattern auto
    CaptGr(const Pattern auto& fn, const Handler auto& handler) {
        return [tpl = std::make_tuple(fn, handler)]
        (MemStream& stream, Captures& captures, UserData usr_val) -> Match {
            Captures
                inner_captures;
            Match
                mCur    = std::get<1>(tpl)(
                            stream,
                            std::get<0>(tpl)(
                                stream,
                                inner_captures,
                                usr_val),
                            inner_captures,
                            usr_val);
            if (mCur) {
                captures.emplace_back(
                    mCur, capt_tag,
                    std::move(inner_captures));
            }

            return mCur;
        };
    }

    template<UserData capt_tag = {}>
    inline Pattern auto
    Capt(const Pattern auto& fn) {
        return [fn]
        (MemStream& stream, Captures& captures, UserData usr_val) -> Match {
            Match
                mCur    = fn(stream, captures, usr_val);
            if (mCur) {
                captures.emplace_back(
                    mCur, capt_tag);
            }

            return mCur;
        };
    }

    template<UserData capt_tag>
    inline Pattern auto
    ReplaceCaptTag() {
        return []
        (MemStream& stream, Captures& captures, UserData) -> Match {
            Match
                mDummy  = { stream.GetPos(), 0uz, false };
            if (!captures.empty()) {
                captures.back().tag = capt_tag;
                mDummy.Toggle();
            }

            return mDummy;
        };
    }

    template<UserData capt_tag>
    inline Pattern auto
    CaptMerge(const Pattern auto& fn) {
        return [fn]
        (MemStream& stream, Captures& captures, UserData usr_val) -> Match {
            Match
                mCur    = fn(stream, captures, usr_val);
            if (mCur) {
                if (!captures.empty()) {
                    Capture&
                        captLast    = captures.back();
                    captLast.tag    = capt_tag;
                    captLast.match  += mCur;
                }
                else
                    mCur.Toggle();
            }

            return mCur;
        };
    }

    template<FixedString... args>
        requires (sizeof...(args) != 0)
    inline constexpr Pattern auto
    Dict() {
        static const Dictionary
            dict = { args... };
        return []
        (MemStream& stream, Captures&, UserData) -> Match {
            intptr_t
                iBegin  = stream.GetPos(),
                iEnd    = iBegin;
            Dictionary::DictMatch
                dm      = dict.StartMatch();
            std::optional<char>
                optc    = {};
            while ((bool)(optc = stream.Read())) {
                dm      = dict.NextMatch(dm, *optc);
                if (!dm) {
                    if (iBegin == iEnd) {
                        iEnd = stream.GetPos();
                        return Match{ iBegin, iEnd, false };
                    }
                    else
                        break;
                }
                if (dm.AtLeafSegmentEnd())
                    iEnd = stream.GetPos();
            }

            stream.SetPos(iEnd);
            return Match{ iBegin, iEnd, iBegin != iEnd };
        };
    }

    template<FixedString... args>
        requires (sizeof...(args) != 0)
    inline constexpr Pattern auto
    NegDict(const Pattern auto& fn) {
        static const Dictionary
            dict = { args... };
        return [fn]
        (MemStream& stream, Captures& captures, UserData usr_val) -> Match {
            Match
                m   = fn(stream, captures, usr_val);
            if (m) {
                std::string_view
                    strvMatch   = m.GetStringView(stream);
                const char*
                    lpLastMatch = nullptr;
                Dictionary::DictMatch
                    dm          = dict.StartMatch();
                for (size_t i = 0; i < strvMatch.size(); ++i) {
                    dm  = dict.NextMatch(dm, strvMatch[i]);
                    if (!dm)
                        return m;
                    else if (dm.AtLeafSegmentEnd())
                        lpLastMatch = &strvMatch[i];
                }

                const char*
                    lpMatchEnd  = strvMatch.data() + strvMatch.size();
                if (lpLastMatch + 1 == lpMatchEnd)
                    m.Toggle();
            }

            return m;
        };
    }


    namespace __impl {
        template<size_t n>
        struct TaggedFixedString :
            public FixedString<n>
        {
            constexpr TaggedFixedString(const char (&szData)[n], UserData capt_tag) :
                FixedString<n>(szData),
                capt_tag(capt_tag) {}

            constexpr UserData
            tag() const noexcept {
                return this->capt_tag;
            }

            UserData
                capt_tag;
        };
    }

    template<__impl::TaggedFixedString... args>
        requires (sizeof...(args) != 0)
    inline constexpr Pattern auto
    CaptDict() {
        static constexpr UserData
            lpCaptTags[]    = { args.tag()... };
        static const Dictionary
            dict            = { args... };

        return []
        (MemStream& stream, Captures& captures, UserData) -> Match {
            intptr_t
                iBegin  = stream.GetPos(),
                iEnd    = iBegin;

            Dictionary::DictMatch
                dm      = dict.StartMatch();
            std::optional<char>
                optc    = {};
            uhalfptr_t
                uWordID = 0;
            while ((bool)(optc = stream.Read())) {
                dm      = dict.NextMatch(dm, *optc);
                if (!dm) {
                    if (iBegin == iEnd) {
                        iEnd = stream.GetPos();
                        return Match{ iBegin, iEnd, false };
                    }
                    else
                        break;
                }
                if (dm.AtLeafSegmentEnd()) {
                    iEnd    = stream.GetPos();
                    uWordID = dm.WordID();
                }
            }

            stream.SetPos(iEnd);
            if (iBegin != iEnd) {
                Match
                    mCur    = { iBegin, iEnd, true };

                assert(uWordID > 0 && uWordID <= sizeof...(args));
                captures.emplace_back(
                    mCur, lpCaptTags[uWordID - 1]);

                return mCur;
            }
            else
                return Match{ iBegin, iEnd, false };
        };
    };

    inline Pattern auto
    LookAhead(const Pattern auto& fn) {
        return [fn]
        (MemStream& stream, Captures& captures, UserData usr_val) -> Match {
            intptr_t
                iBegin  = stream.GetPos();
            size_t
                uCptCnt = captures.size();
            Match
                mCur    = fn(stream, captures, usr_val);
            __impl::RestoreState(iBegin, uCptCnt, stream, captures);
            return Match(iBegin, 0uz, mCur.Good());
        };
    }

    inline Pattern auto
    Not(const Pattern auto& fn) {
        return [fn]
        (MemStream& stream, Captures& captures, UserData usr_val) -> Match {
            Match
                mCur    = fn(stream, captures, usr_val);
            mCur.Toggle();
            return mCur;
        };
    }

    using CRule =
        Match(*)(MemStream&, Captures&, UserData);

    template<CRule fn>
    inline Pattern auto
    Fn() {
        return []
        (MemStream& s, Captures& g, UserData u) -> Match {
            return fn(s, g, u);
        };
    }

    using CHandler =
        Match(*)(MemStream&, Match, CaptureView, UserData);

    template<CHandler fn>
    inline Handler auto
    Fn() {
        return []
        (MemStream& s, Match m, CaptureView c, UserData u) -> Match {
            return fn(s, m, c, u);
        };
    }

    inline Match
    Eval(const Pattern auto& fn, MemStream& stream, Captures& captures, UserData usr_val = {}) {
        return fn(stream, captures, usr_val);
    }

    inline Match
    Eval(const Pattern auto& fn, MemStream& stream, UserData usr_val = {}) {
        Captures
            captures;
        return Eval(fn, stream, captures, usr_val);
    }
}