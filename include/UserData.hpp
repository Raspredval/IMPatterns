#pragma once
static_assert(__cplusplus >= 202302L, "requires C++23 minimum version");

#include <limits>
#include <cstdint>
#include <type_traits>

namespace imp {
    using uhalfptr_t =
        std::conditional_t<sizeof(void*) == 8,
            uint32_t, uint16_t>;

    static constexpr auto
        uhalfptr_min = std::numeric_limits<uhalfptr_t>::min(),
        uhalfptr_max = std::numeric_limits<uhalfptr_t>::max();

    using ihalfptr_t =
        std::conditional_t<sizeof(void*) == 8,
            int32_t, int16_t>;

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