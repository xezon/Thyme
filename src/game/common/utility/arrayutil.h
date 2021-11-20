/**
 * @file
 *
 * @author xezon
 *
 * @brief Array utility functions.
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
#include "stringutil.h"
#include "unicodestring.h"
#include <captainslog.h>

namespace rts
{

// clang-format off

template<typename ValueType, std::size_t Size> inline constexpr array_view<ValueType>
    stack_array_view(ValueType (&array)[Size])
{
    return array_view<ValueType>(array, Size);
}

// char arrays are passed on with size minus one intentionally.
// It is a conservative approach to allow generous algorithms that may place null terminator at
// array_view.data() + array_view.size() as opposed to at array_view.data() + array_view.size() - 1.

template<std::size_t Size> inline constexpr array_view<char>
    stack_array_view(char (&cstring)[Size])
{
    return array_view<char>(cstring, Size - 1);
}

template<std::size_t Size> inline constexpr array_view<const char>
    stack_array_view(const char (&cstring)[Size])
{
    return array_view<const char>(cstring, Size - 1);
}

template<std::size_t Size> inline constexpr array_view<unichar_t>
    stack_array_view(unichar_t (&cstring)[Size])
{
    return array_view<unichar_t>(cstring, Size - 1);
}

template<std::size_t Size> inline constexpr array_view<const unichar_t>
    stack_array_view(const unichar_t (&cstring)[Size])
{
    return array_view<const unichar_t>(cstring, Size - 1);
}

template<typename ValueType> inline constexpr array_view<ValueType>
    make_array_view(ValueType *begin, std::size_t size)
{
    return array_view<ValueType>(begin, size);
}

template<typename ValueType> inline constexpr array_view<ValueType>
    make_array_view(ValueType *begin, ValueType *end)
{
    return array_view<ValueType>(begin, end);
}

// !!! There is no make_array_view for mutable Utf8String and Utf16String. This is intentional.
// Writing to string buffer directly is not good practice, due the reference counted nature of this string.
// All modifications must take place on unique instance only. Use resized_array_view function instead!

// Create array_view<> from const Utf8String.
inline array_view<const typename Utf8String::value_type>
    make_array_view(const Utf8String &instance)
{
    return array_view<const typename Utf8String::value_type>(instance.Str(), instance.Get_Length());
}

// Create array_view<> from const Utf16String.
inline array_view<const typename Utf16String::value_type>
    make_array_view(const Utf16String &instance)
{
    return array_view<const typename Utf16String::value_type>(instance.Str(), instance.Get_Length());
}

namespace detail
{
template <typename CharType, typename ObjectType, typename SizeType> inline array_view<CharType>
    utfstring_resized_array_view(ObjectType &object, SizeType size)
{
    using object_type = ObjectType;
    using char_type = CharType;
    using size_type = SizeType;

    captainslog_assert(size >= 0);
    if (size == 0) {
        return array_view<char_type>();
    }
    CHAR_TRAITS_CONSTEXPR char_type null_char = get_char<char_type>('\0');
    object_type copy = object;
    char_type* peek = object.Get_Buffer_For_Read(size); // Allocates + 1
    const char_type* reader = copy.Str();
    const char_type* end = peek + size;
    char_type* inserter = peek;
    // Copy original string as far as it goes.
    while (*reader != null_char && inserter != end) {
        *inserter++ = *reader++;
    }
    // Fill the rest with null.
    while (inserter != end + 1) {
        *inserter++ = null_char;
    }
    return array_view<char_type>(peek, size);
}
}

// Create array_view<> from Utf8String and resize the string with the given size.
inline array_view<typename Utf8String::value_type>
    resized_array_view(Utf8String &utfstring, typename Utf8String::size_type size)
{
    return detail::utfstring_resized_array_view<typename Utf8String::value_type>(utfstring, size);
}

// Create array_view<> from Utf16String and resize the string with the given size.
inline array_view<typename Utf16String::value_type>
    resized_array_view(Utf16String &utfstring, typename Utf16String::size_type size)
{
    return detail::utfstring_resized_array_view<typename Utf16String::value_type>(utfstring, size);
}

inline constexpr array_view<char>
    make_array_view(char* cstring)
{
    return array_view<char>(cstring, strlen(cstring));
}

inline constexpr array_view<const char>
    make_array_view(const char* cstring)
{
    return array_view<const char>(cstring, strlen(cstring));
}

inline constexpr array_view<unichar_t>
    make_array_view(unichar_t* cstring)
{
    return array_view<unichar_t>(cstring, u_strlen(cstring));
}

inline constexpr array_view<const unichar_t>
    make_array_view(const unichar_t* cstring)
{
    return array_view<const unichar_t>(cstring, u_strlen(cstring));
}

// clang-format on

} // namespace rts
