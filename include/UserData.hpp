#pragma once
static_assert(__cplusplus >= 202302L, "requires C++23 minimum version");

#include <cstdint>
#include <type_traits>

namespace imp {
    using uhalfptr_t =
        std::conditional_t<sizeof(void*) == 8,
            uint32_t, uint16_t>;

    using ihalfptr_t =
        std::conditional_t<sizeof(void*) == 8,
            int32_t, int16_t>;

    union UserData {
        uintptr_t   u;
        intptr_t    i;
        void*       p;
        struct {
            uhalfptr_t ul, uh;
        };
        struct {
            ihalfptr_t il, ih;
        };

        constexpr UserData() = default;

        constexpr UserData(uintptr_t u) :
            u{u} {}

        constexpr UserData(intptr_t i) :
            i{i} {}

        constexpr UserData(void* p) :
            p{p} {}

        constexpr UserData(uhalfptr_t ul, uhalfptr_t uh) :
            ul{ul}, uh{uh} {}

        constexpr UserData(ihalfptr_t il, ihalfptr_t ih) :
            il{il}, ih{ih} {}
    };
}