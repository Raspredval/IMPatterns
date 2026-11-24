#pragma once
static_assert(__cplusplus >= 202302L, "requires C++23 minimum version");

#include <limits>
#include <cstdint>

namespace imp {
    template<std::size_t nbits>
    struct SizedTypes {};

    template<>
    struct SizedTypes<8> {
        using i = int8_t;
        using u = uint8_t;
    };

    template<>
    struct SizedTypes<16> {
        using i = int16_t;
        using u = uint16_t;
    };

    template<>
    struct SizedTypes<32> {
        using i = int32_t;
        using u = uint32_t;
        using f = float;
    };

    template<>
    struct SizedTypes<64> {
        using i = int64_t;
        using u = uint64_t;
        using f = double;
    };

    using uhalfptr_t =
        typename SizedTypes<sizeof(void*) * 4>::u;

    static constexpr auto
        uhalfptr_min = std::numeric_limits<uhalfptr_t>::min(),
        uhalfptr_max = std::numeric_limits<uhalfptr_t>::max();

    using ihalfptr_t =
        typename SizedTypes<sizeof(void*) * 4>::i;

    static constexpr auto
        ihalfptr_min = std::numeric_limits<ihalfptr_t>::min(),
        ihalfptr_max = std::numeric_limits<ihalfptr_t>::max();
}