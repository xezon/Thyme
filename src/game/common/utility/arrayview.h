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

#include "asciistring.h"
#include "unicodestring.h"
#include <cstddef>

namespace rts
{
// #FEATURE array_view allows to pass along a buffer and its size in one go. Alternative is std::span<> with c++20.
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

    constexpr array_view() : m_begin(nullptr), m_end(nullptr) noexcept {}
    constexpr array_view(pointer begin, size_type size) : m_begin(begin), m_end(begin + size) {}
    constexpr array_view(pointer begin, pointer end) : m_begin(begin), m_end(end) {}

    constexpr array_view(const array_view &) noexcept = default;
    constexpr array_view &operator=(const array_view &) noexcept = default;

    constexpr reference operator[](size_type index) const { return m_begin[index]; }
    constexpr reference operator*() const { return *m_begin; }
    constexpr pointer operator->() const noexcept { return m_begin; }

    constexpr operator bool() const noexcept { return m_begin != nullptr; }

    constexpr iterator begin() const noexcept { return m_begin; }
    constexpr iterator end() const noexcept { return m_end; }

    constexpr reference front() const { return *m_begin; }
    constexpr reference back() const { return *(m_end - 1); }

    constexpr pointer data() const noexcept { return m_begin; }

    // Get the size of the array.
    constexpr size_type size() const noexcept { return m_end - m_begin; }
    constexpr size_type size_bytes() const noexcept { return size() * sizeof(value_type); }

    constexpr bool empty() const noexcept { return size() == 0; }

private:
    pointer m_begin;
    pointer m_end;
};

// clang-format off

template<typename ValueType, std::size_t Size> constexpr array_view<ValueType>
    stack_array_view(ValueType (&array)[Size])
{
    return array_view<ValueType>(array, Size);
}

template<typename ValueType> constexpr array_view<ValueType>
    make_array_view(ValueType *begin, std::size_t size)
{
    return array_view<ValueType>(begin, size);
}

template<typename ValueType> constexpr array_view<ValueType>
    make_array_view(ValueType *begin, ValueType *end)
{
    return array_view<ValueType>(begin, end);
}

template<typename ContainerType> constexpr array_view<typename ContainerType::value_type>
    make_array_view(ContainerType &instance)
{
    using view_type = array_view<typename ContainerType::value_type>;
    return view_type(instance.data(), instance.size());
}

template<typename ContainerType> constexpr array_view<const typename ContainerType::value_type>
    make_array_view(const ContainerType &instance)
{
    using view_type = array_view<const typename ContainerType::value_type>;
    return view_type(instance.data(), instance.size());
}

// Create array_view<> from container and resize the container with the given size. Mostly useful for Utf8String and
// Utf16String, because these classes have no concept of size and thus cannot hold size themselves.
template<typename ContainerType> constexpr array_view<typename ContainerType::value_type>
    resized_string_view(ContainerType &instance, typename ContainerType::size_type size)
{
    using view_type = array_view<typename ContainerType::value_type>;
    instance.resize(size);
    return view_type(instance.data(), size);
}

template<std::size_t Size> constexpr array_view<char>
    stack_string_view(char (&string)[Size])
{
    return array_view<char>(string, Size - 1);
}

template<std::size_t Size> constexpr array_view<const char>
    stack_string_view(const char (&string)[Size])
{
    return array_view<const char>(string, Size - 1);
}

template<std::size_t Size> constexpr array_view<unichar_t>
    stack_string_view(unichar_t (&string)[Size])
{
    return array_view<unichar_t>(string, Size - 1);
}

template<std::size_t Size> constexpr array_view<const unichar_t>
    stack_string_view(const unichar_t (&string)[Size])
{
    return array_view<const unichar_t>(string, Size - 1);
}

// Create array_view<> from Utf8String. Typically useful for reads only.
constexpr array_view<typename Utf8String::value_type>
    make_string_view(Utf8String &instance)
{
    using view_type = array_view<typename Utf8String::value_type>;
    return view_type(instance.data(), instance.Get_Length());
}

// Create array_view<> from Utf8String (const). Typically useful for reads only.
constexpr array_view<const typename Utf8String::value_type>
    make_string_view(const Utf8String &instance)
{
    using view_type = array_view<const typename Utf8String::value_type>;
    return view_type(instance.data(), instance.Get_Length());
}

// Create array_view<> from Utf16String. Typically useful for reads only.
constexpr array_view<typename Utf16String::value_type>
    make_string_view(Utf16String &instance)
{
    using view_type = array_view<typename Utf16String::value_type>;
    return view_type(instance.data(), instance.Get_Length());
}

// Create array_view<> from Utf16String (const). Typically useful for reads only.
constexpr array_view<const typename Utf16String::value_type>
    make_string_view(const Utf16String &instance)
{
    using view_type = array_view<const typename Utf16String::value_type>;
    return view_type(instance.data(), instance.Get_Length());
}

constexpr array_view<char>
    make_string_view(char* string)
{
    return array_view<char>(string, string + strlen(string));
}

constexpr array_view<const char>
    make_string_view(const char* string)
{
    return array_view<const char>(string, string + strlen(string));
}

constexpr array_view<unichar_t>
    make_string_view(unichar_t* string)
{
    return array_view<unichar_t>(string, string + u_strlen(string));
}

constexpr array_view<const unichar_t>
    make_string_view(const unichar_t* string)
{
    return array_view<const unichar_t>(string, string + u_strlen(string));
}

// clang-format on

} // namespace rts
