#pragma once
static_assert(__cplusplus >= 202002L, "requires C++23 minimum version");

#include <span>
#include <vector>
#include "Match.hpp"

namespace imp {
    struct Capture {
        using uhalfptr_t =
            std::conditional_t<sizeof(void*) == 8,
                uint32_t, uint16_t>;

        static constexpr size_t
            uhalfptr_off = sizeof(void*) * 4UL;

        imp::Match  m;
        uintptr_t   u;

        inline constexpr friend std::pair<uhalfptr_t, uhalfptr_t>
        to_halfptr(uintptr_t u) noexcept {
            return {
                (uhalfptr_t)(u),
                (uhalfptr_t)(u >> uhalfptr_off)
            };
        }

        inline constexpr friend uintptr_t
        from_halfptr(uhalfptr_t h, uhalfptr_t l) noexcept {
            return ((uintptr_t)l) | ((uintptr_t)h << uhalfptr_off);
        }
    };

    using Captures =
        std::vector<Capture>;
    using CapturesList =
        std::vector<Captures>;
    using CapturesView =
        std::span<const Capture>;
}