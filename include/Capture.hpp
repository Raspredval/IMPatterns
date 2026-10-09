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

        inline void
        setL(uhalfptr_t uLow) noexcept {
            uintptr_t
                uVal    = uLow,
                uMask   = (1UL << uhalfptr_off) - 1;
            this->u     = (this->u & uMask) | uVal;
        }

        inline void
        setH(uhalfptr_t uHigh) noexcept {
            uintptr_t
                uVal    = uHigh,
                uMask   = ((1UL << uhalfptr_off) - 1) << uhalfptr_off;
            this->u     = (this->u & uMask) | (uVal << uhalfptr_off);
        }

        inline uhalfptr_t
        getL() const noexcept {
            return (uhalfptr_t)(this->u);
        }

        inline uhalfptr_t
        getH() const noexcept {
            return (uhalfptr_t)(this->u >> uhalfptr_off);
        }
    };

    using Captures =
        std::vector<Capture>;
    using CapturesList =
        std::vector<Captures>;
    using CapturesView =
        std::span<const Capture>;
}