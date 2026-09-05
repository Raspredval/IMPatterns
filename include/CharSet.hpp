#pragma once
#include <cstddef>
#include <cstdint>

namespace imp {
    struct CharSet {
        template<size_t n>
        constexpr CharSet(const char (&lpcSet)[n]) noexcept {
            for (char c : lpcSet)
                this->insert(c);
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