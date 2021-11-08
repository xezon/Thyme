/**
 * @file
 *
 * @author xezon
 *
 * @brief Lightweight custom string view catered to Thyme.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "arrayview.h"
#include "asciistring.h"
#include "unicodestring.h"

namespace rts
{
// For now string_view<> is just an array_view<>, thus has limited functionality. Can be changed when necessary, ideally
// using std::string_view<> with c++17.

template<typename CharType> using basic_string_view = array_view<CharType>;
using string_view = basic_string_view<char>;
using ustring_view = basic_string_view<unichar_t>;

// clang-format off

// Create string_view<> from container and resize the container with the given size. Mostly useful for Utf8String and
// Utf16String, because these classes have no concept of size and thus cannot hold size themselves.
template<typename ContainerType> constexpr basic_string_view<typename ContainerType::value_type>
    resized_string_view(ContainerType &instance, typename ContainerType::size_type size)
{
    using view_type = basic_string_view<typename ContainerType::value_type>;
    instance.resize(size);
    return view_type(instance.data(), size);
}

template<std::size_t Size> constexpr basic_string_view<char>
    stack_string_view(char (&string)[Size])
{
    return basic_string_view<char>(string, Size - 1);
}

template<std::size_t Size> constexpr basic_string_view<const char>
    stack_string_view(const char (&string)[Size])
{
    return basic_string_view<const char>(string, Size - 1);
}

template<std::size_t Size> constexpr basic_string_view<unichar_t>
    stack_string_view(unichar_t (&string)[Size])
{
    return basic_string_view<unichar_t>(string, Size - 1);
}

template<std::size_t Size> constexpr basic_string_view<const unichar_t>
    stack_string_view(const unichar_t (&string)[Size])
{
    return basic_string_view<const unichar_t>(string, Size - 1);
}

// ATTENTION: There is no make_string_view for mutable Utf8String and Utf16String. This is intentional.
// Writing to string buffer directly is not good practice, due the reference counted nature of this string.
// All modifications must take place on unique instance only. Use resized_string_view function instead!

// Create string_view<> from const Utf8String.
constexpr basic_string_view<const typename Utf8String::value_type>
    make_string_view(const Utf8String &instance)
{
    using view_type = basic_string_view<const typename Utf8String::value_type>;
    return view_type(instance.data(), instance.Get_Length());
}

// Create string_view<> from const Utf16String.
constexpr basic_string_view<const typename Utf16String::value_type>
    make_string_view(const Utf16String &instance)
{
    using view_type = basic_string_view<const typename Utf16String::value_type>;
    return view_type(instance.data(), instance.Get_Length());
}

constexpr basic_string_view<char>
    make_string_view(char* string)
{
    return basic_string_view<char>(string, string + strlen(string));
}

constexpr basic_string_view<const char>
    make_string_view(const char* string)
{
    return basic_string_view<const char>(string, string + strlen(string));
}

constexpr basic_string_view<unichar_t>
    make_string_view(unichar_t* string)
{
    return basic_string_view<unichar_t>(string, string + u_strlen(string));
}

constexpr basic_string_view<const unichar_t>
    make_string_view(const unichar_t* string)
{
    return basic_string_view<const unichar_t>(string, string + u_strlen(string));
}

// clang-format on

} // namespace rts
