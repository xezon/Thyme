/**
 * @file
 *
 * @author xezon
 *
 * @brief Lightweight custom array view specialized for Thyme structures.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "captainslog.h"
#include "stringutil.h"
#include <cstddef>

// #FEATURE array_view allows to pass along a buffer and its size in one go.
// Alternative is std::span<> with c++20
template<typename ValueType> class array_view
{
public:
    using element_type = ValueType;
    using value_type = ValueType;
    using size_type = std::ptrdiff_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = value_type *;
    using const_iterator = const value_type *;

    ~array_view() noexcept = default;

    template<size_type Size> constexpr static array_view create(value_type (&buf)[Size]) noexcept
    {
        return array_view(buf, Size);
    }
    constexpr static array_view create(value_type *begin, value_type *end) { return array_view(begin, end); }
    constexpr static array_view create(value_type *begin, size_type size) { return array_view(begin, size); }

    constexpr array_view() : m_begin(nullptr), m_end(nullptr) noexcept {}
    constexpr array_view(value_type *begin, size_type size) : m_begin(begin), m_end(begin + size)
    {
        captainslog_assert(m_end >= m_begin);
    }
    constexpr array_view(value_type *begin, value_type *end) : m_begin(begin), m_end(end)
    {
        captainslog_assert(m_end >= m_begin);
    }

    constexpr array_view(const array_view &) noexcept = default;
    constexpr array_view &operator=(const array_view &) noexcept = default;

    constexpr reference operator[](size_type index) { return m_begin[index]; }
    constexpr reference operator*() { return *m_begin; }
    constexpr pointer operator->() noexcept { return m_begin; }

    constexpr const_reference operator[](size_type index) const { return m_begin[index]; }
    constexpr const_reference operator*() const { return *m_begin; }
    constexpr const_pointer operator->() const noexcept { return m_begin; }

    constexpr operator bool() const noexcept { return m_begin != nullptr; }

    constexpr iterator begin() noexcept { return m_begin; }
    constexpr iterator end() noexcept { return m_end; }
    constexpr const_iterator begin() const noexcept { return m_begin; }
    constexpr const_iterator cbegin() const noexcept { return m_begin; }
    constexpr const_iterator end() const noexcept { return m_end; }
    constexpr const_iterator cend() const noexcept { return m_end; }

    constexpr reference front() { return *m_begin; }
    constexpr reference back() { return *(m_end - 1); }
    constexpr const_reference front() const { return *m_begin; }
    constexpr const_reference back() const { return *(m_end - 1); }

    constexpr pointer data() noexcept { return m_begin; }
    constexpr const_pointer data() const noexcept { return m_begin; }

    // Get the size of the array.
    constexpr size_type capacity() const noexcept { return m_end - m_begin; }
    constexpr size_type size() const noexcept { return m_end - m_begin; }
    constexpr size_type size_bytes() const noexcept { return size() * sizeof(value_type); }
    constexpr bool empty() const noexcept { return size() == 0; }

    // Get the size of the array or the null terminated string length if value type is char, unichar_t.
    // This function is always safe to call even if the string is not null terminated.
    constexpr size_type length() const noexcept { return size(); }

private:
    value_type *m_begin;
    value_type *m_end;
};

template<> constexpr array_view<char>::size_type array_view<char>::length() const noexcept
{
    constexpr value_type null_char = rts::Get_Null<value_type>();

    for (value_type *it = m_begin; it != m_end; ++it)
        if (*it == null_char)
            return (it - m_begin);

    return size();
}

template<> constexpr array_view<unichar_t>::size_type array_view<unichar_t>::length() const noexcept
{
    constexpr value_type null_char = rts::Get_Null<value_type>();

    for (value_type *it = m_begin; it != m_end; ++it)
        if (*it == null_char)
            return (it - m_begin);

    return size();
}
