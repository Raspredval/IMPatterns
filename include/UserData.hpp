#pragma once
static_assert(__cplusplus >= 202302L, "requires C++23 minimum version");

#include <limits>
#include <cstdint>

namespace imp {
    namespace __impl {
        template<std::size_t nbits>
        struct sized_types {};

        template<>
        struct sized_types<8> {
            using i = int8_t;
            using u = uint8_t;
        };

        template<>
        struct sized_types<16> {
            using i = int16_t;
            using u = uint16_t;
        };

        template<>
        struct sized_types<32> {
            using i = int32_t;
            using u = uint32_t;
            using f = float;
        };

        template<>
        struct sized_types<64> {
            using i = int64_t;
            using u = uint64_t;
            using f = double;
        };
    }

    using uhalfptr_t =
        typename __impl::sized_types<sizeof(void*) * 4>::u;

    static constexpr auto
        uhalfptr_min = std::numeric_limits<uhalfptr_t>::min(),
        uhalfptr_max = std::numeric_limits<uhalfptr_t>::max();

    using ihalfptr_t =
        typename __impl::sized_types<sizeof(void*) * 4>::i;

    static constexpr auto
        ihalfptr_min = std::numeric_limits<ihalfptr_t>::min(),
        ihalfptr_max = std::numeric_limits<ihalfptr_t>::max();

    union UserData {
        uintptr_t   u;
        intptr_t    i;
        void*       ptr;
        struct upair {
            uhalfptr_t l, h;
        } upr;
        struct ipair {
            ihalfptr_t l, h;
        } ipr;

        constexpr UserData() = default;

        constexpr UserData(uintptr_t u) :
            u{u} {}

        constexpr UserData(intptr_t i) :
            i{i} {}

        constexpr UserData(void* p) :
            ptr{p} {}

        constexpr UserData(uhalfptr_t l, uhalfptr_t h) :
            upr{l, h} {}

        constexpr UserData(ihalfptr_t l, ihalfptr_t h) :
            ipr{l, h} {}
    };
}