/**
 * @file
 *
 * @author xezon
 *
 * @brief Lightweight custom array view catered to Thyme.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include <cstddef>

namespace Thyme
{
namespace rts
{
// #FEATURE array_view allows to pass along a buffer and its size in one go. Alternative is std::span<> with c++20. Not to be
// confused with string_view, which is read and string only!
template<typename ValueType> class array_view
{
public:
    using element_type = ValueType;
    using value_type = ValueType;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = value_type *;
    using const_iterator = const value_type *;

    ~array_view() noexcept = default;

    constexpr array_view() noexcept : m_begin(nullptr), m_size(0) {}
    constexpr array_view(pointer begin, size_type size) :
        m_begin(begin), m_size(size) {} // #TODO throw if size is too large?
    constexpr array_view(pointer begin, pointer end) :
        m_begin(begin), m_size(begin + size) {} // #TODO throw if end is too small?

    template<size_type Size> constexpr array_view(element_type (&arr)[Size]) noexcept : m_begin(arr), m_size(Size) {}

    constexpr array_view(const array_view &) noexcept = default;
    constexpr array_view &operator=(const array_view &) noexcept = default;

    constexpr reference operator[](size_type index) const { return m_begin[index]; }
    constexpr reference operator*() const { return *m_begin; }
    constexpr pointer operator->() const noexcept { return m_begin; }

    constexpr operator bool() const noexcept { return m_begin != nullptr; }

    constexpr iterator begin() const noexcept { return m_begin; }
    constexpr iterator end() const noexcept { return m_begin + m_size; }

    constexpr reference front() const { return *m_begin; }
    constexpr reference back() const { return *(m_begin + m_size - 1); }

    constexpr pointer data() const noexcept { return m_begin; }

    constexpr size_type size() const noexcept { return m_size; }
    constexpr size_type size_bytes() const noexcept { return m_size * sizeof(element_type); }

    constexpr bool empty() const noexcept { return m_size == 0; }

private:
    pointer m_begin;
    size_type m_size;
};

} // namespace rts
} // namespace Thyme
