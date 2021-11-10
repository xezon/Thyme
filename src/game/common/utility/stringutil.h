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

template<typename CharType> constexpr bool is_null_or_whitespace(CharType ch)
{
    return is_null(ch) || is_whitespace(ch);
}

enum StripOption
{
    STRIP_DEFAULT = 0,
    STRIP_REPLACE_WHITESPACE = BIT(0), // Stripping will replace all whitespace with space character.
    STRIP_LEADING_AND_TRAILING_ONLY = BIT(1), // Stripping within a sentence is prevented.
};
DEFINE_ENUMERATION_BITWISE_OPERATORS(StripOption);

// Strips leading, trailing and duplicate whitespace. Can provide options to customize the stripping result. Returns count of
// stripped characters. Writes null over all stripped characters at the end. Compatible with UTF-8 and UTF-16.
template<typename StringType> std::size_t strip_obsolete_whitespace(StringType &string, StripOption option = STRIP_DEFAULT)
{
    captainslog_assert(string.end() >= string.begin());
    captainslog_assert((string.end() - string.begin()) < 0x0fff);

    if (string.empty())
        return 0;

    using char_type = typename StringType::value_type;
    constexpr char_type null_char = get_char<char_type>('\0');
    constexpr char_type space_char = get_char<char_type>(' ');
    const char_type *reader = string.begin();
    const char_type *reader_end = string.end();
    char_type *writer = string.begin();

    const bool replace_whitespace = (option & STRIP_REPLACE_WHITESPACE) != 0;
    const bool leading_and_trailing_only = (option & STRIP_LEADING_AND_TRAILING_ONLY) != 0;

    // Increment reader to first non-whitespace character.
    for (; reader != reader_end && is_null_or_whitespace(*reader); ++reader) {
    }

    // Decrement reader_end to first non-whitespace character.
    for (; reader != reader_end && is_null_or_whitespace(*(reader_end - 1)); --reader_end) {
    }

    while (reader != reader_end) {
        char_type curr_char = *reader;
        char_type next_char = (++reader != reader_end) ? *reader : null_char;
        const bool curr_char_is_space = is_whitespace(curr_char);

        // If this and next character is any whitespace or null, then skip.
        if (!leading_and_trailing_only && curr_char_is_space && is_null_or_whitespace(next_char)) {
            continue;
        }

        // If this character is any whitespace, then turn it into a space character.
        if (replace_whitespace && curr_char_is_space) {
            curr_char = space_char;
        }

        // Write out.
        *writer++ = curr_char;
    }

    const std::size_t stripped_count = ((reader - writer) + (string.end() - reader_end));

    // Fill the rest with null.
    while (writer != string.end()) {
        *writer++ = null_char;
    }

    return stripped_count;
}

// Simplified algorithm for null terminated strings. Strips leading, trailing and duplicate whitespace. Returns count of
// stripped characters. Writes null over all stripped characters at the end. Compatible with UTF-8 and UTF-16.
template<typename CharType> std::size_t strip_all_obsolete_whitespace(CharType *cstring)
{
    using char_type = CharType;

    constexpr char_type null_char = get_char<char_type>('\0');
    constexpr char_type space_char = get_char<char_type>(' ');
    const char_type *reader = cstring;
    char_type *writer = cstring;

    // Iterate to first non-whitespace character.
    for (; *reader != null_char && is_whitespace(*reader); ++reader) {
    }

    while (*reader != null_char) {
        char_type curr_char = *reader;
        char_type next_char = *++reader;
        const bool curr_char_is_space = is_whitespace(curr_char);

        // If this and next character is any whitespace or null, then skip.
        if (curr_char_is_space && is_null_or_whitespace(next_char)) {
            continue;
        }

        // If this character is any whitespace, then turn it into a space character.
        if (curr_char_is_space) {
            curr_char = space_char;
        }

        // Write out.
        *writer++ = curr_char;
    }

    const std::size_t stripped_count = (reader - writer);

    // Fill the rest with null. Reader is the end by now.
    while (writer != reader) {
        *writer++ = null_char;
    }

    return stripped_count;
}

// Replace string characters by given search sequence with replacement character.
template<typename CharType> std::size_t replace_characters(CharType *cstring, const CharType *search, CharType replace)
{
    using char_type = CharType;

    constexpr char_type null_char = get_char<char_type>('\0');
    char_type *writer = cstring;
    std::size_t replaced_count = 0;

    while (*writer != null_char) {
        const char_type *reader = search;
        while (*reader != null_char) {
            if (*writer == *reader) {
                *writer = replace;
                replaced_count++;
                break;
            }
            ++reader;
        }
        ++writer;
    }

    return replaced_count;
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
                // Write out the read character.
                *writer++ = escaped_char.real;
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

    // Write null character.
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

    // Write null character.
    *writer = null_char;

    const std::size_t num_copied = (writer - dest);

    return num_copied;
}

} // namespace rts
