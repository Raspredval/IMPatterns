#pragma once
#include <cstddef>
#include <cstdint>

namespace imp {
    struct CharSet {
        template<size_t n>
            requires (n > 1)
        constexpr CharSet(const char (&lpcSet)[n]) noexcept {
            for (size_t i = 0; i != (n - 1); ++i) {
                this->insert(lpcSet[i]);
            }
        }

        constexpr void
        insert(char c) {
            uint64_t
                uc  = (uint64_t)(c);
            this->lpMap[uc >> 6] |= (uint64_t)1 << (uc & 63);
        }

        constexpr void
        remove(char c) noexcept {
            uint64_t
                uc  = (uint64_t)(c);
            this->lpMap[uc >> 6] &= ~((uint64_t)1 << (uc & 63));
        }

        constexpr bool
        contains(char c) const noexcept {
            uint64_t
                uc  = (uint64_t)(c);
            return (this->lpMap[uc >> 6] & ((uint64_t)1 << (uc & 63))) != 0;
        }

        uint64_t
            lpMap[4] = {0,0,0,0};
    };
}