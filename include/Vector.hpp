#pragma once
static_assert(__cplusplus >= 202302L, "requires C++23 minimum version");

#include <memory>
#include <cassert>

namespace imp {
    template<typename T>
    class Vector {
    public:
        using value_type =
            std::remove_reference_t<T>;
        using iterator  =
            value_type*;
        using const_iterator =
            const value_type*;

    private:
        struct Header {
            const size_t
                uCapacity;
            size_t
                uSize;
            value_type
                lpData[];
        };

        static Header*
        allocHeader(size_t uAtLeast, size_t uInitSize = 0) {
            static constexpr size_t
                uMinCapacity    = std::max<size_t>(1, 64 / sizeof(value_type));

            size_t
                uCapacity   = uMinCapacity;
            while (uCapacity < uAtLeast)
                uCapacity *= 2;

            std::byte*
                lpStorage   = new
                    (std::align_val_t{alignof(value_type)})
                    std::byte[sizeof(Header) + sizeof(value_type) * uCapacity];
            return std::construct_at(
                (Header*)lpStorage,
                uCapacity, uInitSize
            );
        }

        static Header*
        reallocHeader(Header* lpCur, size_t uAtLeast) {
            assert(lpCur);

            if (lpCur->uCapacity < uAtLeast) {
                Header*
                    lpResult    = allocHeader(uAtLeast, lpCur->uSize);
                for (size_t i = 0; i != lpCur->uSize; ++i) {
                    std::construct_at(
                        &lpResult->lpData[i],
                        std::move(lpCur->lpData[i])
                    );

                    std::destroy_at(
                        &lpCur->lpData[i]
                    );
                }

                std::destroy_at(lpCur);
                delete[] (std::byte*)lpCur;
                lpCur   = lpResult;
            }

            return lpCur;
        }

    public:
        Vector() = default;

        Vector(Vector&& obj) noexcept {
            this->lpHeader  = obj.lpHeader;
            obj.lpHeader    = nullptr;
        }

        Vector(const Vector& obj) {
            if (obj.lpHeader) {
                this->lpHeader  = allocHeader(
                                    obj.lpHeader->uCapacity,
                                    obj.lpHeader->uSize);
                for (size_t i = 0; i != this->lpHeader->uSize; ++i) {
                    std::construct_at(
                        &this->lpHeader->lpData[i],
                        obj.lpHeader->lpData[i]
                    );
                }
            }
        }

        ~Vector() {
            if (this->lpHeader) {
                for (size_t i = 0; i != this->lpHeader->uSize; ++i) {
                    std::destroy_at(&this->lpHeader->lpData[i]);
                }

                std::destroy_at(this->lpHeader);
                delete[] (std::byte*)this->lpHeader;
            }
        }

        auto&
        operator=(Vector&& obj) noexcept {
            Vector
                temp    = std::move(obj);
            std::swap(
                this->lpHeader,
                temp.lpHeader
            );

            return *this;
        }

        auto&
        operator=(const Vector& obj) {
            Vector
                temp    = obj;
            std::swap(
                this->lpHeader,
                temp.lpHeader
            );

            return *this;
        }

        size_t
        size() const noexcept {
            return (this->lpHeader)
                ? this->lpHeader->uSize : 0;
        }

        bool
        empty() const noexcept {
            return this->lpHeader == nullptr ||
                this->lpHeader->uSize == 0;
        }

        size_t
        capacity() const noexcept {
            return (this->lpHeader)
                ? this->lpHeader->uCapacity : 0;
        }

        const value_type*
        data() const noexcept {
            return (this->lpHeader)
                ? this->lpHeader->lpData : nullptr;
        }

        value_type*
        data() noexcept {
            return (this->lpHeader)
                ? this->lpHeader->lpData : nullptr;
        }

        const value_type&
        operator[](size_t uIndex) const noexcept {
            return this->data()[uIndex];
        }

        value_type&
        operator[](size_t uIndex) noexcept {
            return this->data()[uIndex];
        }

        const_iterator
        begin() const noexcept {
            return (this->lpHeader)
                ? this->lpHeader->lpData : nullptr;
        }

        const_iterator
        end() const noexcept {
            return (this->lpHeader)
                ? this->lpHeader->lpData +
                    this->lpHeader->uSize
                : nullptr;
        }

        iterator
        begin() noexcept {
            return (this->lpHeader)
                ? this->lpHeader->lpData : nullptr;
        }

        iterator
        end() noexcept {
            return (this->lpHeader)
                ? this->lpHeader->lpData +
                    this->lpHeader->uSize
                : nullptr;
        }

        const_iterator
        cbegin() const noexcept {
            return (this->lpHeader)
                ? this->lpHeader->lpData : nullptr;
        }

        const_iterator
        cend() const noexcept {
            return (this->lpHeader)
                ? this->lpHeader->lpData +
                    this->lpHeader->uSize
                : nullptr;
        }

        template<typename... Args>
            requires std::constructible_from<value_type, Args...>
        value_type&
        emplace_back(Args&&... args) {
            if (!this->lpHeader)
                this->lpHeader  = allocHeader(1);
            else
                this->lpHeader  = reallocHeader(
                                    this->lpHeader,
                                    this->lpHeader->uSize + 1);
            return *std::construct_at(
                &this->lpHeader->lpData[
                    this->lpHeader->uSize++],
                std::forward<Args>(args)...
            );
        }

        value_type&
        push_back(const value_type& obj) {
            return this->emplace_back(obj);
        }

        value_type&
        push_back(value_type&& obj) {
            return this->emplace_back(std::move(obj));
        }

        void
        pop_back() {
            assert(!this->empty());

            std::destroy_at(
                &this->lpHeader->lpData[
                    this->lpHeader->uSize -= 1]
            );
        }

        const value_type&
        back() const noexcept {
            assert(!this->empty());

            return this->lpHeader->lpData[this->lpHeader->uSize - 1];
        }

        const value_type&
        front() const noexcept {
            assert(!this->empty());

            return this->lpHeader->lpData[0];
        }

        value_type&
        back() noexcept {
            assert(!this->empty());

            return this->lpHeader->lpData[this->lpHeader->uSize - 1];
        }

        value_type&
        front() noexcept {
            assert(!this->empty());

            return this->lpHeader->lpData[0];
        }

    private:
        Header*
            lpHeader    = nullptr;
    };
}