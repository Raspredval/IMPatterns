#pragma once
static_assert(__cplusplus >= 202302L, "requires C++23 minimum version");

#include <concepts>

#include "Match.hpp"
#include "Vector.hpp"
#include "CharSet.hpp"
#include "MemStream.hpp"
#include "Dictionary.hpp"
#include "FixedString.hpp"

#define IMP_DECL_RULE(NAME) imp::Match NAME (imp::MemStream& s, imp::Captures<IMP_CAPTTAG>& c, IMP_USERDATA d)
#define IMP_MAKE_RULE(NAME, FN) IMP_DECL_RULE(NAME) { return (FN)(s, c, d); }
#define IMP_DECL_HANDLER(NAME) imp::Match NAME ([[maybe_unused]] imp::MemStream& stream, [[maybe_unused]] imp::Match m, [[maybe_unused]] imp::CaptureView<IMP_CAPTTAG> captures, [[maybe_unused]] IMP_USERDATA usr_val)
#define IMP_MAKE_HANDLER(NAME, DEF) IMP_DECL_HANDLER(NAME) DEF

namespace imp {
    template<typename CaptTag>
    struct Capture {
        Match
            match;
        CaptTag
            tag;
        Vector<Capture>
            children;
    };

    template<typename CaptTag>
    using Captures =
        Vector<Capture<CaptTag>>;

    template<typename CaptTag>
    using CaptureView =
        std::span<const Capture<CaptTag>>;

    template<typename Fn, typename CaptTag, typename UserData>
    concept Pattern =
        std::is_class_v<Fn> &&
        std::same_as<std::invoke_result_t<const Fn, MemStream&, Captures<CaptTag>&, UserData>, Match>;

    template<typename Fn, typename CaptTag, typename UserData>
        concept Handler =
            std::is_class_v<Fn> &&
            std::same_as<std::invoke_result_t<const Fn, MemStream&, Match, CaptureView<CaptTag>, UserData>, Match>;

    namespace __impl {
        template<typename CaptTag>
        inline void
        RestoreState(intptr_t iOldPos, size_t uCptCnt, MemStream& stream, Captures<CaptTag>& captures) noexcept {
            stream.SetPos(iOldPos);
            while (captures.size() > uCptCnt)
                captures.pop_back();
        }

        template<typename CaptTag, size_t n>
        struct TaggedFixedString :
            public FixedString<n>
        {
            consteval TaggedFixedString(const char (&szData)[n], const CaptTag& capt_tag) :
                FixedString<n>(szData),
                capt_tag(capt_tag) {}

            consteval const CaptTag&
            tag() const noexcept {
                return this->capt_tag;
            }

            CaptTag
                capt_tag;
        };
    }

    template<typename CaptTag, typename UserData>
    struct Patterns {
        template<typename Fn>
        struct __wrapper : public Fn {};

        template<Pattern<CaptTag, UserData> Lhs, Pattern<CaptTag, UserData> Rhs>
        friend Pattern<CaptTag, UserData> auto
        constexpr operator>>(const __wrapper<Lhs>& lhs, const __wrapper<Rhs>& rhs) {
            return __wrapper{[tpl = std::make_tuple(lhs, rhs)]
            (MemStream& stream, Captures<CaptTag>& captures, UserData usr_val) -> Match {
                Match
                    mLhs    = std::get<0>(tpl)(stream, captures, usr_val);
                if (mLhs)
                    mLhs += std::get<1>(tpl)(stream, captures, usr_val);
                return mLhs;
            }};
        }

        template<Pattern<CaptTag, UserData> Lhs, Pattern<CaptTag, UserData> Rhs>
        friend Pattern<CaptTag, UserData> auto
        constexpr operator|(const __wrapper<Lhs>& lhs, const __wrapper<Rhs>& rhs) {
            return __wrapper{[tpl = std::make_tuple(lhs, rhs)]
            (MemStream& stream, Captures<CaptTag>& captures, UserData usr_val) -> Match {
                intptr_t
                    iBegin  = stream.GetPos();
                size_t
                    uCptCnt = captures.size();
                Match
                    mLhs    = std::get<0>(tpl)(stream, captures, usr_val);
                if (!mLhs) {
                    __impl::RestoreState(iBegin, uCptCnt, stream, captures);
                    mLhs    = std::get<1>(tpl)(stream, captures, usr_val);
                }

                return mLhs;
            }};
        }

        template<Pattern<CaptTag, UserData> Lhs, Handler<CaptTag, UserData> Rhs>
        friend Pattern<CaptTag, UserData> auto
        constexpr operator/(const __wrapper<Lhs>& fn, const __wrapper<Rhs>& handler) {
            return __wrapper{[tpl = std::make_tuple(fn, handler)]
            (MemStream& stream, Captures<CaptTag>& captures, UserData usr_val) -> Match {
                return std::get<1>(tpl)(
                    stream,
                    std::get<0>(tpl)(
                        stream,
                        captures,
                        usr_val),
                    captures,
                    usr_val);
            }};
        }

        template<FixedString strMatch>
        static Pattern<CaptTag, UserData> auto
        constexpr Str() {
            return __wrapper{[]
            (MemStream& stream, Captures<CaptTag>&, UserData) -> Match {
                intptr_t
                    iBegin  = stream.GetPos();
                for (size_t i = 0; i != strMatch.size(); ++i) {
                    auto optc   = stream.Read();
                    if (!optc || strMatch[i] != *optc)
                        return Match(iBegin, i, false);
                }

                return Match(iBegin, strMatch.size());
            }};
        }

        template<CharSet chset>
        static Pattern<CaptTag, UserData> auto
        constexpr Set() {
            return __wrapper{[]
            (MemStream& stream, Captures<CaptTag>&, UserData) -> Match {
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
            }};
        }

        template<CharSet chset>
        static Pattern<CaptTag, UserData> auto
        constexpr NegSet() {
            return __wrapper{[]
            (MemStream& stream, Captures<CaptTag>&, UserData) -> Match {
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
            }};
        }

        static Pattern<CaptTag, UserData> auto
        constexpr AnyChar() {
            return __wrapper{[]
            (MemStream& stream, Captures<CaptTag>&, UserData) -> Match {
                intptr_t
                    iBegin  = stream.GetPos();
                return ((bool)stream.Read())
                    ? Match(iBegin, 1uz, true)
                    : Match(iBegin, 0uz, false);
            }};
        }

        static Pattern<CaptTag, UserData> auto
        constexpr NoChars() {
            return __wrapper{[]
            (MemStream& stream, Captures<CaptTag>&, UserData) -> Match {
                intptr_t
                    iBegin  = stream.GetPos();
                return ((bool)stream.Read())
                    ? Match(iBegin, 1uz, false)
                    : Match(iBegin, 0uz, true);
            }};
        }

        template<size_t n>
            requires (n > 0)
        static Pattern<CaptTag, UserData> auto
        constexpr UpTo(const Pattern<CaptTag, UserData> auto& fn) {
            return __wrapper{[fn]
            (MemStream& stream, Captures<CaptTag>& captures, UserData usr_val) -> Match {
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
            }};
        }

        template<size_t n>
        static Pattern<CaptTag, UserData> auto
        constexpr AtLeast(const Pattern<CaptTag, UserData> auto& fn) {
            return __wrapper{[fn]
            (MemStream& stream, Captures<CaptTag>& captures, UserData usr_val) -> Match {
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
            }};
        }

        template<size_t n>
            requires (n > 0)
        static Pattern<CaptTag, UserData> auto
        constexpr Exactly(const Pattern<CaptTag, UserData> auto& fn) {
            return __wrapper{[fn]
            (MemStream& stream, Captures<CaptTag>& captures, UserData usr_val) -> Match {
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
            }};
        }

        static Pattern<CaptTag, UserData> auto
        constexpr Opt(const Pattern<CaptTag, UserData> auto& fn) {
            return UpTo<1>(fn);
        }

        static Pattern<CaptTag, UserData> auto
        constexpr Any(const Pattern<CaptTag, UserData> auto& fn) {
            return AtLeast<0>(fn);
        }

        static Pattern<CaptTag, UserData> auto
        constexpr Some(const Pattern<CaptTag, UserData> auto& fn) {
            return AtLeast<1>(fn);
        }

        template<CaptTag capt_tag = {}>
        static Pattern<CaptTag, UserData> auto
        constexpr CaptGr(const Pattern<CaptTag, UserData> auto& fn) {
            return __wrapper{[fn]
            (MemStream& stream, Captures<CaptTag>& captures, UserData usr_val) -> Match {
                Captures<CaptTag>
                    inner_captures;
                Match
                    mCur    = fn(stream, inner_captures, usr_val);
                if (mCur) {
                    captures.emplace_back(
                        mCur, capt_tag,
                        std::move(inner_captures));
                }

                return mCur;
            }};
        }

        template<CaptTag capt_tag = {}>
        static Pattern<CaptTag, UserData> auto
        constexpr CaptGr(const Pattern<CaptTag, UserData> auto& fn, const Handler<CaptTag, UserData> auto& handler) {
            return __wrapper{[tpl = std::make_tuple(fn, handler)]
            (MemStream& stream, Captures<CaptTag>& captures, UserData usr_val) -> Match {
                Captures<CaptTag>
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
            }};
        }

        template<CaptTag capt_tag = {}>
        static Pattern<CaptTag, UserData> auto
        constexpr Capt(const Pattern<CaptTag, UserData> auto& fn) {
            return __wrapper{[fn]
            (MemStream& stream, Captures<CaptTag>& captures, UserData usr_val) -> Match {
                Match
                    mCur    = fn(stream, captures, usr_val);
                if (mCur) {
                    captures.emplace_back(
                        mCur, capt_tag);
                }

                return mCur;
            }};
        }

        template<CaptTag capt_tag>
        static Pattern<CaptTag, UserData> auto
        CaptMerge(const Pattern<CaptTag, UserData> auto& fn) {
            return __wrapper{[fn]
            (MemStream& stream, Captures<CaptTag>& captures, UserData usr_val) -> Match {
                Captures<CaptTag>
                    inner_captures;
                Match
                    mCur    = fn(stream, inner_captures, usr_val);
                if (mCur) {
                    if (!captures.empty()) {
                        Capture<CaptTag>&
                            captLast    = captures.back();
                        captLast.tag    = capt_tag;
                        captLast.match  += mCur;
                        for (auto& c : inner_captures) {
                            captLast.children
                                .push_back(std::move(c));
                        }
                    }
                    else
                        mCur.Toggle();
                }

                return mCur;
            }};
        }

        template<FixedString... args>
            requires (sizeof...(args) != 0)
        static Pattern<CaptTag, UserData> auto
        constexpr Dict() {
            static const Dictionary
                dict = { args... };

            return __wrapper{[]
            (MemStream& stream, Captures<CaptTag>&, UserData) -> Match {
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
            }};
        }

        template<FixedString... args>
            requires (sizeof...(args) != 0)
        static Pattern<CaptTag, UserData> auto
        constexpr NegDict(const Pattern<CaptTag, UserData> auto& fn) {
            static const Dictionary
                dict = { args... };

            return __wrapper{[fn]
            (MemStream& stream, Captures<CaptTag>& captures, UserData usr_val) -> Match {
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
            }};
        }

        template<__impl::TaggedFixedString... args>
            requires (sizeof...(args) != 0)
        static Pattern<CaptTag, UserData> auto
        constexpr CaptDict() {
            static constexpr CaptTag
                lpCaptTags[]    = { args.tag()... };
            static const Dictionary
                dict            = { args... };

            return __wrapper{[]
            (MemStream& stream, Captures<CaptTag>& captures, UserData) -> Match {
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
            }};
        };

        static Pattern<CaptTag, UserData> auto
        constexpr LookAhead(const Pattern<CaptTag, UserData> auto& fn) {
            return __wrapper{[fn]
            (MemStream& stream, Captures<CaptTag>& captures, UserData usr_val) -> Match {
                intptr_t
                    iBegin  = stream.GetPos();
                size_t
                    uCptCnt = captures.size();
                Match
                    mCur    = fn(stream, captures, usr_val);
                __impl::RestoreState(iBegin, uCptCnt, stream, captures);
                return Match(iBegin, 0uz, mCur.Good());
            }};
        }

        static Pattern<CaptTag, UserData> auto
        constexpr Not(const Pattern<CaptTag, UserData> auto& fn) {
            return [fn]
            (MemStream& stream, Captures<CaptTag>& captures, UserData usr_val) -> Match {
                Match
                    mCur    = fn(stream, captures, usr_val);
                mCur.Toggle();
                return mCur;
            };
        }

        using CRule =
            Match(*)(MemStream&, Captures<CaptTag>&, UserData);

        template<CRule fn>
        static Pattern<CaptTag, UserData> auto
        constexpr Fn() {
            return __wrapper{[]
            (MemStream& s, Captures<CaptTag>& g, UserData u) -> Match {
                return fn(s, g, u);
            }};
        }

        using CHandler =
            Match(*)(MemStream&, Match, CaptureView<CaptTag>, UserData);

        template<CHandler fn>
        static Handler<CaptTag, UserData> auto
        constexpr Fn() {
            return __wrapper{[]
            (MemStream& s, Match m, CaptureView<CaptTag> c, UserData u) -> Match {
                return fn(s, m, c, u);
            }};
        }

        static Match
        constexpr Eval(const Pattern<CaptTag, UserData> auto& fn, MemStream& stream, Captures<CaptTag>& captures, UserData usr_val = {}) {
            return fn(stream, captures, usr_val);
        }

        static Match
        constexpr Eval(const Pattern<CaptTag, UserData> auto& fn, MemStream& stream, UserData usr_val = {}) {
            Captures<CaptTag>
                captures;
            return Eval(fn, stream, captures, usr_val);
        }
    };
}