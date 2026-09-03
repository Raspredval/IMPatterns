#pragma once
static_assert(__cplusplus >= 202002L, "requires C++23 minimum version");

#include <algorithm>

namespace imp {
    template<size_t n>
        requires (n != 0)
    struct FixedString {
        constexpr FixedString(const char (&szData)[n]) {
            std::ranges::copy(szData, this->lpcData);
        }

        static constexpr size_t
        size() noexcept {
            return n - 1;
        }

        static constexpr bool
        empty() noexcept {
            return FixedString::size() == 0;
        }

        constexpr const char*
        c_str() const noexcept {
            return this->lpcData;
        }

        constexpr const char*
        begin() const noexcept {
            return this->c_str();
        }

        constexpr const char*
        end() const noexcept {
            return this->c_str() + FixedString::size();
        }

        constexpr bool
        contains(char c) const noexcept {
            for (size_t i = 0; i != FixedString::size(); ++i) {
                if (this->lpcData[i] == c)
                    return true;
            }

            return false;
        }

        constexpr char
        operator[](size_t uIndex) const noexcept {
            return this->lpcData[uIndex];
        }

        char
            lpcData[n];
    };
}