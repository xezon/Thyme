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
#include "sizedinteger.h"
#include <string>
#include <unichar.h>

#if defined(THYME_USE_STLPORT)
#define CHAR_TRAITS_CONSTEXPR
#else
#define CHAR_TRAITS_CONSTEXPR constexpr
#endif

namespace rts
{

// Get a single character. For use in templates.
template<typename CharType> inline CHAR_TRAITS_CONSTEXPR CharType get_char(const char ch);

template<> inline CHAR_TRAITS_CONSTEXPR char get_char<char>(const char ch)
{
    return ch;
}

template<> inline CHAR_TRAITS_CONSTEXPR unichar_t get_char<unichar_t>(const char ch)
{
    const int i_ch = std::char_traits<char>::to_int_type(ch);
    const unichar_t u_ch = std::char_traits<unichar_t>::to_char_type(i_ch);
    return u_ch;
}

// Get an empty null terminated c string. For use in templates.
template<typename CharType> inline constexpr const CharType *get_null_str();

template<> inline constexpr const char *get_null_str<char>()
{
    return "";
}

template<> inline constexpr const unichar_t *get_null_str<unichar_t>()
{
    return U_CHAR("");
}

// Ascii whitespace is any of these
// ' ' (0x20)(SPC)space
// '\t'(0x09)(TAB)horizontal tab
// '\n'(0x0a)(LF)newline
// '\v'(0x0b)(VT)vertical tab
// '\f'(0x0c)(FF)feed
// '\r'(0x0d)(CR)carriage return
template<typename CharType> CHAR_TRAITS_CONSTEXPR bool is_asciiwhitespace(CharType ch)
{
    return ch == get_char<CharType>(' ') || ch == get_char<CharType>('\t') || ch == get_char<CharType>('\n')
        || ch == get_char<CharType>('\v') || ch == get_char<CharType>('\f') || ch == get_char<CharType>('\r');
}

template<typename CharType> CHAR_TRAITS_CONSTEXPR bool is_null(CharType ch)
{
    return ch == get_char<CharType>('\0');
}

template<typename CharType> CHAR_TRAITS_CONSTEXPR bool is_space(CharType ch)
{
    return ch == get_char<CharType>(' ');
}

template<typename CharType> CHAR_TRAITS_CONSTEXPR bool is_null_or_asciiwhitespace(CharType ch)
{
    return is_null(ch) || is_asciiwhitespace(ch);
}

template<typename CharType> CHAR_TRAITS_CONSTEXPR std::size_t strlen_tpl(const CharType *src)
{
    using char_type = CharType;
    const char_type null_char = get_char<char_type>('\0');
    const char_type *end = src;
    while (*end != null_char) {
        ++end;
    }
    return (end - src);
}

template<typename CharType> CHAR_TRAITS_CONSTEXPR int strcmp_tpl(const CharType *s1, const CharType *s2)
{
    using char_type = CharType;
    using unsigned_type = typename unsigned_integer_for_size<sizeof(CharType)>::type;
    const char_type null_char = get_char<char_type>('\0');

    while (*s1 != null_char && *s1 == *s2) {
        ++s1;
        ++s2;
    }
    const auto i1 = *reinterpret_cast<const unsigned_type *>(s1);
    const auto i2 = *reinterpret_cast<const unsigned_type *>(s2);
    return (i1 > i2) - (i2 > i1);
}

template<typename CharType> CHAR_TRAITS_CONSTEXPR int strncmp_tpl(const CharType *s1, const CharType *s2, std::size_t count)
{
    using char_type = CharType;
    using unsigned_type = typename unsigned_integer_for_size<sizeof(CharType)>::type;
    const char_type null_char = get_char<char_type>('\0');

    while (count != 0 && *s1 != null_char && *s1 == *s2) {
        ++s1;
        ++s2;
        --count;
    }
    if (count == 0) {
        return 0;
    } else {
        const auto i1 = *reinterpret_cast<const unsigned_type *>(s1);
        const auto i2 = *reinterpret_cast<const unsigned_type *>(s2);
        return (i1 > i2) - (i2 > i1);
    }
}

// Strips leading and trailing spaces. Returns length of new string after strip. Writes null over all stripped characters at
// the end. Compatible with UTF-8 and UTF-16.
template<typename CharType> std::size_t strip_leading_and_trailing_spaces(CharType *dest)
{
    using char_type = CharType;

    const char_type null_char = get_char<char_type>('\0');
    const char_type *reader = dest;
    const char_type *reader_end = dest + strlen_tpl(dest);
    char_type *writer = dest;
    const char_type *writer_end = reader_end;

    for (; reader != reader_end && is_space(*reader); ++reader) {
    }

    for (; reader != reader_end && is_space(*(reader_end - 1)); --reader_end) {
    }

    while (reader != reader_end) {
        *writer++ = *reader++;
    }

    const std::size_t len = writer - dest;

    while (writer != writer_end) {
        *writer++ = null_char;
    }

    return len;
}

// Strips leading, trailing and duplicate spaces. Preserves other whitespace characters such as LF and strips surrounding
// spaces. Returns length of new string after strip. Writes null over all stripped characters at the end. Compatible with
// UTF-8 and UTF-16.
template<typename CharType> std::size_t strip_obsolete_spaces(CharType *dest)
{
    using char_type = CharType;

    const char_type null_char = get_char<char_type>('\0');
    char_type prev_char = get_char<char_type>(' ');
    const char_type *reader = dest;
    char_type *writer = dest;

    for (; !is_null(*reader) && is_space(*reader); ++reader) {
    }

    while (!is_null(*reader)) {
        char_type curr_char = *reader;
        char_type next_char = *++reader;

        if (is_space(curr_char) && (is_null_or_asciiwhitespace(next_char) || is_asciiwhitespace(prev_char))) {
            continue;
        }

        *writer++ = curr_char;
        prev_char = curr_char;
    }

    const std::size_t len = (writer - dest);

    while (writer != reader) {
        *writer++ = null_char;
    }

    return len;
}

// Replaces string characters by given search characters with replacement character. Compatible with UTF-16.
// Compatible with UTF-8 if search and replace are ASCII characters.
template<typename CharType> void replace_characters(CharType *dest, const CharType *search, CharType replace)
{
    using char_type = CharType;

    char_type *writer = dest;

    for (; !is_null(*writer); ++writer) {
        const char_type *searcher = search;

        for (; !is_null(*searcher); ++searcher) {
            if (*writer == *searcher) {
                *writer = replace;
                break;
            }
        }
    }
}

// Replaces string characters by given search character sequence with replacement character sequence. Returns count of
// characters copied to destination string, not including null terminator Compatible with UTF-16. Compatible with UTF-8 if
// search and replace are ASCII characters.
template<typename CharType>
std::size_t replace_character_sequence(
    CharType *dest, std::size_t size, const CharType *src, const CharType *search, const CharType *replace)
{
    using char_type = CharType;

    const char_type null_char = get_char<char_type>('\0');
    const std::size_t search_len = strlen_tpl(search);
    const std::size_t replace_len = strlen_tpl(replace);
    const char_type *reader = src;
    const char_type *writer_end = dest + size - 1;
    char_type *writer = dest;
    std::size_t replace_count = 0;

    while (*reader != null_char && writer != writer_end) {
        if (replace_count > 0) {
            *writer++ = *(replace + replace_len - replace_count);
            if (--replace_count == 0) {
                reader += search_len;
            }
        } else if (strncmp_tpl(reader, search, search_len) == 0) {
            replace_count = replace_len;
        } else {
            *writer++ = *reader++;
        }
    }

    *writer = null_char;

    return (writer - dest);
}

// Strips string characters by given search characters. Compatible with UTF-16. Compatible with UTF-8 if search and replace
// are ASCII characters.
template<typename CharType> void strip_characters(CharType *dest, const CharType *search)
{
    using char_type = CharType;

    const char_type null_char = get_char<char_type>('\0');
    const char_type *reader = dest;
    char_type *writer = dest;

    for (; !is_null(*reader); ++reader) {
        bool skip = false;
        const char_type *searcher = search;

        for (; !is_null(*searcher); ++searcher) {
            if (*reader == *searcher) {
                skip = true;
                break;
            }
        }

        if (skip) {
            continue;
        }

        *writer++ = *reader;
    }

    while (writer != reader) {
        *writer++ = null_char;
    }
}

template<typename CharType> struct escaped_char_alias
{
    static CHAR_TRAITS_CONSTEXPR escaped_char_alias make_real_alias2(char real, char alias1, char alias2)
    {
        return { get_char<CharType>(real), get_char<CharType>(alias1), get_char<CharType>(alias2) };
    }

    CharType real;
    CharType alias[2];
};

template<typename CharType> using escaped_char_alias_view = array_view<const escaped_char_alias<CharType>>;

template<typename CharType> escaped_char_alias_view<CharType> get_standard_escaped_characters()
{
    static CHAR_TRAITS_CONSTEXPR escaped_char_alias<CharType> escaped_chars[] = {
        escaped_char_alias<CharType>::make_real_alias2('\0', '\\', '0'),
        escaped_char_alias<CharType>::make_real_alias2('\a', '\\', 'a'),
        escaped_char_alias<CharType>::make_real_alias2('\b', '\\', 'b'),
        escaped_char_alias<CharType>::make_real_alias2('\t', '\\', 't'),
        escaped_char_alias<CharType>::make_real_alias2('\n', '\\', 'n'),
        escaped_char_alias<CharType>::make_real_alias2('\v', '\\', 'v'),
        escaped_char_alias<CharType>::make_real_alias2('\f', '\\', 'f'),
        escaped_char_alias<CharType>::make_real_alias2('\r', '\\', 'r'),
    };
    return escaped_char_alias_view<CharType>(escaped_chars);
}

// Converts from escaped characters, meaning a 2 character sequence is converted to a 1 character sequence. Returns count of
// characters copied to destination string, not including null terminator. Writes null at the end. Compatible with UTF-16.
// Compatible with UTF-8 if escaped character symbols are ASCII only.
template<typename CharType>
std::size_t convert_from_escaped_characters(CharType *dest,
    std::size_t size,
    const CharType *src,
    escaped_char_alias_view<CharType> escaped_chars_view = get_standard_escaped_characters<CharType>())
{
    using char_type = CharType;

    const char_type null_char = get_char<char_type>('\0');
    const char_type *reader = src;
    const char_type *writer_end = dest + size - 1;
    char_type *writer = dest;

    while (*reader != null_char && writer != writer_end) {
        char_type curr_char = *reader;
        char_type next_char = *++reader;

        bool done = false;

        for (const escaped_char_alias<char_type> &escaped_char : escaped_chars_view) {
            if (curr_char == escaped_char.alias[0] && next_char == escaped_char.alias[1]) {
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

    return (writer - dest);
}

// Converts to escaped characters, meaning a 1 character sequence is converted to a 2 character sequence. Returns count of
// characters copied to destination string, not including null terminator. Writes null at the end. Compatible with UTF-16.
// Compatible with UTF-8 if escaped character symbols are ASCII only.
template<typename CharType>
std::size_t convert_to_escaped_characters(CharType *dest,
    std::size_t size,
    const CharType *src,
    escaped_char_alias_view<CharType> escaped_chars_view = get_standard_escaped_characters<CharType>())
{
    using char_type = CharType;

    const char_type null_char = rts::get_char<char_type>('\0');
    const char_type *reader = src;
    const char_type *writer_end = dest + size - 1;
    char_type *writer = dest;

    while (*reader != null_char && writer != writer_end) {
        char_type curr_char = *reader++;

        bool done = false;

        for (const escaped_char_alias<char_type> &escaped_char : escaped_chars_view) {
            if (curr_char == escaped_char.real) {
                // Write out the escaped character sequence.
                *writer++ = escaped_char.alias[0];
                if (writer != writer_end) {
                    *writer++ = escaped_char.alias[1];
                }
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

    return (writer - dest);
}

} // namespace rts
