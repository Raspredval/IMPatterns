#pragma once
static_assert(__cplusplus >= 202302L, "requires C++23 minimum version");

#include <array>
#include <string_view>

namespace imp {
    namespace __impl {
        template<size_t n>
        inline std::array<char, n>
        constexpr makeArray(const char (&szData)[n]) noexcept {
            std::array<char, n>
                arrResult;
            for (size_t i = 0; i != n; ++i)
                arrResult[i] = szData[i];

            return arrResult;
        }
    }

    template<size_t n>
        requires (n > 1)
    struct FixedString {
        consteval FixedString(const char (&szData)[n]) :
            arrData{__impl::makeArray(szData)} {}

        static constexpr size_t
        size() noexcept {
            return n - 1;
        }

        constexpr const char*
        c_str() const noexcept {
            return this->arrData.data();
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
                if (this->arrData[i] == c)
                    return true;
            }

            return false;
        }

        explicit constexpr
        operator std::string_view() const noexcept {
            return { this->c_str(), this->size() };
        }

        constexpr char
        operator[](size_t uIndex) const noexcept {
            return this->arrData[uIndex];
        }

        const std::array<char, n>
            arrData;
    };
}