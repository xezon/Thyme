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

#include "stringutil.h"
#include <cstddef>
#include <string.h>

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

    ~array_view() = default;

    template<size_type Size> constexpr static array_view Create(value_type (&buf)[Size]) { return array_view(buf, Size); }
    constexpr static array_view Create(value_type *begin, value_type *end) { return array_view(begin, end); }
    constexpr static array_view Create(value_type *begin, size_type size) { return array_view(begin, size); }

    constexpr array_view() : m_begin(nullptr), m_end(nullptr) {}
    constexpr array_view(value_type *begin, size_type size) : m_begin(begin), m_end(begin + size) {}
    constexpr array_view(value_type *begin, value_type *end) : m_begin(begin), m_end(end) {}

    constexpr array_view(const array_view &) = default;
    constexpr array_view &operator=(const array_view &) = default;

    constexpr reference operator[](size_type index) { return m_begin[index]; }
    constexpr reference operator*() { return *m_begin; }
    constexpr pointer operator->() { return m_begin; }

    constexpr const_reference operator[](size_type index) const { return m_begin[index]; }
    constexpr const_reference operator*() const { return *m_begin; }
    constexpr const_pointer operator->() const { return m_begin; }

    constexpr operator bool() const { return m_begin != nullptr; }

    constexpr iterator begin() { return m_begin; }
    constexpr iterator end() { return m_end; }
    constexpr const_iterator begin() const { return m_begin; }
    constexpr const_iterator cbegin() const { return m_begin; }
    constexpr const_iterator end() const { return m_end; }
    constexpr const_iterator cend() const { return m_end; }

    constexpr reference front() { return *m_begin; }
    constexpr reference back() { return *(m_end - 1); }
    constexpr const_reference front() const { return *m_begin; }
    constexpr const_reference back() const { return *(m_end - 1); }

    constexpr pointer data() { return m_begin; }
    constexpr const_pointer data() const { return m_begin; }

    // Same as data(). Synonymous for function in game string class.
    constexpr pointer Peek() { return m_begin; }
    constexpr const_pointer Peek() const { return m_begin; }

    // Get the size of the array.
    constexpr size_type size() const { return m_end - m_begin; }
    constexpr size_type size_bytes() const { return size() * sizeof(value_type); }
    constexpr bool empty() const { return size() == 0; }

    // Get the size of the array or the null terminated string length if value type is char, unichar_t.
    // This function is always safe to call even if the string is not null terminated.
    constexpr size_type length() const { return size(); }

    // Same as length(). Synonymous for function in game string class.
    constexpr size_type Get_Length() const { return size(); }

private:
    value_type *m_begin;
    value_type *m_end;
};

template<> constexpr array_view<char>::size_type array_view<char>::length() const
{
    constexpr value_type null_char = rts::Get_Null<value_type>();

    for (value_type *it = m_begin; it != m_end; ++it)
        if (*it == null_char)
            return (it - m_begin);

    return size();
}

template<> constexpr array_view<unichar_t>::size_type array_view<unichar_t>::length() const
{
    constexpr value_type null_char = rts::Get_Null<value_type>();

    for (value_type *it = m_begin; it != m_end; ++it)
        if (*it == null_char)
            return (it - m_begin);

    return size();
}
