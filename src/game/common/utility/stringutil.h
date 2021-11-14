/**
 * @file
 *
 * @author xezon
 *
 * @brief String utility functions.
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
#include <captainslog.h>
#include <locale>
#include <macros.h>
#include <unichar.h>

namespace Thyme
{
namespace rts
{
template<typename CharType> constexpr CharType get_char(char ch);

template<> constexpr char get_char<char>(char ch)
{
    return ch;
}

template<> constexpr unichar_t get_char<unichar_t>(char ch)
{
    return static_cast<unichar_t>(static_cast<unsigned char>(ch));
}

// clang-format off

// Whitespace is any of these:
// ' ' (0x20)space (SPC)
// '\t'(0x09)horizontal tab (TAB)
// '\n'(0x0a)newline (LF)
// '\v'(0x0b)vertical tab (VT)
// '\f'(0x0c)feed (FF)
// '\r'(0x0d)carriage return (CR)
template<typename CharType> constexpr bool is_whitespace(CharType ch)
{
#ifdef THYME_USE_STLPORT
    return ch == get_char<CharType>(' ')
        || ch == get_char<CharType>('\t')
        || ch == get_char<CharType>('\n')
        || ch == get_char<CharType>('\v')
        || ch == get_char<CharType>('\f')
        || ch == get_char<CharType>('\r');
#else
    return std::isspace(ch, std::locale());
#endif
}
// clang-format on

template<typename CharType> constexpr bool is_null(CharType ch)
{
    return ch == get_char<CharType>('\0');
}

template<typename CharType> constexpr bool is_space(CharType ch)
{
    return ch == get_char<CharType>(' ');
}

template<typename CharType> constexpr bool is_null_or_whitespace(CharType ch)
{
    return is_null(ch) || is_whitespace(ch);
}

template<typename CharType> constexpr std::size_t strlen_tpl(const CharType *cstring)
{
    using char_type = CharType;
    constexpr char_type null_char = get_char<char_type>('\0');
    const char_type *end = cstring;
    for (; *end != null_char; ++end) {
    }
    const std::size_t len = (end - cstring);
    return len;
}

// Strips leading and trailing spaces. Returns length of new string after strip. Writes null over all stripped characters at
// the end. Compatible with UTF-8 and UTF-16.
template<typename CharType> std::size_t strip_leading_and_trailing_spaces(CharType *cstring)
{
    using char_type = CharType;

    constexpr char_type null_char = get_char<char_type>('\0');
    const char_type *reader = cstring;
    const char_type *reader_end = cstring + strlen_tpl(cstring);
    char_type *writer = cstring;
    const char_type *writer_end = reader_end;

    for (; reader != reader_end && is_space(*reader); ++reader) {
    }

    for (; reader != reader_end && is_space(*(reader_end - 1)); --reader_end) {
    }

    while (reader != reader_end) {
        *writer++ = *reader++;
    }

    const std::size_t len = writer - cstring;

    while (writer != writer_end) {
        *writer++ = null_char;
    }

    return len;
}

// Strips leading, trailing and duplicate spaces. Preserves other whitespace characters such as LF and strips surrounding
// spaces. Returns length of new string after strip. Writes null over all stripped characters at the end. Compatible with
// UTF-8 and UTF-16.
template<typename CharType> std::size_t strip_obsolete_spaces(CharType *cstring)
{
    using char_type = CharType;

    constexpr char_type null_char = get_char<char_type>('\0');
    char_type prev_char = get_char<char_type>(' ');
    const char_type *reader = cstring;
    char_type *writer = cstring;

    for (; !is_null(*reader) && is_space(*reader); ++reader) {
    }

    while (!is_null(*reader)) {
        char_type curr_char = *reader;
        char_type next_char = *++reader;

        if (is_space(curr_char) && (is_null_or_whitespace(next_char) || is_whitespace(prev_char))) {
            continue;
        }

        *writer++ = curr_char;
        prev_char = curr_char;
    }

    const std::size_t len = (writer - cstring);

    while (writer != reader) {
        *writer++ = null_char;
    }

    return len;
}

// Replace string characters by given search sequence with replacement character.
template<typename CharType> void replace_characters(CharType *cstring, const CharType *search, CharType replace)
{
    using char_type = CharType;

    char_type *writer = cstring;

    while (!is_null(*writer)) {
        const char_type *reader = search;
        while (!is_null(*reader)) {
            if (*writer == *reader) {
                *writer = replace;
                break;
            }
            ++reader;
        }
        ++writer;
    }
}

template<typename CharType> struct escaped_char_alias
{
    CharType real;
    CharType alias1;
    CharType alias2;
};

template<typename CharType> using escaped_char_alias_view = array_view<const escaped_char_alias<CharType>>;

template<typename CharType> escaped_char_alias_view<CharType> get_standard_escaped_characters()
{
    static const escaped_char_alias<CharType> escaped_chars[] = {
        { get_char<CharType>('\0'), get_char<CharType>('\\'), get_char<CharType>('0') },
        { get_char<CharType>('\a'), get_char<CharType>('\\'), get_char<CharType>('a') },
        { get_char<CharType>('\b'), get_char<CharType>('\\'), get_char<CharType>('b') },
        { get_char<CharType>('\t'), get_char<CharType>('\\'), get_char<CharType>('t') },
        { get_char<CharType>('\n'), get_char<CharType>('\\'), get_char<CharType>('n') },
        { get_char<CharType>('\v'), get_char<CharType>('\\'), get_char<CharType>('v') },
        { get_char<CharType>('\f'), get_char<CharType>('\\'), get_char<CharType>('f') },
        { get_char<CharType>('\r'), get_char<CharType>('\\'), get_char<CharType>('r') },
    };
    return escaped_char_alias_view<CharType>(escaped_chars);
}

// Converts from escaped characters, meaning a 2 character sequence is converted to a 1 character sequence. Returns count of
// characters copied to destination string, not including null terminator. Writes null at the end. Compatible with UTF-8 and
// UTF-16, if escaped character symbols are ASCII only.
template<typename CharType>
std::size_t convert_from_escaped_characters(CharType *dest,
    const CharType *src,
    std::size_t size,
    escaped_char_alias_view<CharType> escaped_chars_view = get_standard_escaped_characters<CharType>())
{
    using char_type = CharType;

    constexpr char_type null_char = get_char<char_type>('\0');
    const char_type *reader = src;
    const char_type *writer_end = dest + size - 1;
    char_type *writer = dest;

    while (*reader != null_char && writer != writer_end) {
        char_type curr_char = *reader;
        char_type next_char = *++reader;

        bool done = false;

        for (const escaped_char_alias<char_type> &escaped_char : escaped_chars_view) {
            if (curr_char == escaped_char.alias1 && next_char == escaped_char.alias2) {
                // Write out the real character.
                *writer++ = escaped_char.real;
                ++reader;
                done = true;
                break;
            }
        }

        if (done) {
            continue;
        }

        // Write out the character as is.
        *writer++ = curr_char;
    }

    *writer = null_char;

    const std::size_t num_copied = (writer - dest);

    return num_copied;
}

// Converts to escaped characters, meaning a 1 character sequence is converted to a 2 character sequence. Returns count of
// characters copied to destination string, not including null terminator. Writes null at the end. Compatible with UTF-8 and
// UTF-16, if escaped character symbols are ASCII only.
template<typename CharType>
std::size_t convert_to_escaped_characters(CharType *dest,
    const CharType *src,
    std::size_t size,
    escaped_char_alias_view<CharType> escaped_chars_view = get_standard_escaped_characters<CharType>())
{
    using char_type = CharType;

    constexpr char_type null_char = rts::get_char<char_type>('\0');
    const char_type *reader = src;
    const char_type *writer_end = dest + size - 2; // -2 because we may print up to 2 characters in one iteration.
    char_type *writer = dest;

    while (*reader != null_char && writer != writer_end) {
        char_type curr_char = *reader++;

        bool done = false;

        for (const escaped_char_alias<char_type> &escaped_char : escaped_chars_view) {
            if (curr_char == escaped_char.real) {
                // Write out the escaped character sequence.
                *writer++ = escaped_char.alias1;
                *writer++ = escaped_char.alias2;
                done = true;
                break;
            }
        }

        if (done) {
            continue;
        }

        // Write out the character as is.
        *writer++ = curr_char;
    }

    *writer = null_char;

    const std::size_t num_copied = (writer - dest);

    return num_copied;
}

} // namespace rts
} // namespace Thyme
