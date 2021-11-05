/**
 * @file
 *
 * @author xezon
 *
 * @brief String utility functions and similar
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include <captainslog.h>
#include <locale>
#include <macros.h>
#include <unichar.h>

namespace rts
{
// clang-format off

template<typename CharType> constexpr CharType Get_Null();
template<typename CharType> constexpr CharType Get_Tab();
template<typename CharType> constexpr CharType Get_LF();
template<typename CharType> constexpr CharType Get_VTab();
template<typename CharType> constexpr CharType Get_FF();
template<typename CharType> constexpr CharType Get_CR();
template<typename CharType> constexpr CharType Get_Space();

template<> constexpr char Get_Null<char>()  { return '\0'; }
template<> constexpr char Get_Tab<char>()   { return '\t'; }
template<> constexpr char Get_LF<char>()    { return '\n'; }
template<> constexpr char Get_VTab<char>()  { return '\v'; }
template<> constexpr char Get_FF<char>()    { return '\f'; }
template<> constexpr char Get_CR<char>()    { return '\r'; }
template<> constexpr char Get_Space<char>() { return ' '; }

template<> constexpr unichar_t Get_Null<unichar_t>()  { return U_CHAR('\0'); }
template<> constexpr unichar_t Get_Tab<unichar_t>()   { return U_CHAR('\t'); }
template<> constexpr unichar_t Get_LF<unichar_t>()    { return U_CHAR('\n'); }
template<> constexpr unichar_t Get_VTab<unichar_t>()  { return U_CHAR('\v'); }
template<> constexpr unichar_t Get_FF<unichar_t>()    { return U_CHAR('\f'); }
template<> constexpr unichar_t Get_CR<unichar_t>()    { return U_CHAR('\r'); }
template<> constexpr unichar_t Get_Space<unichar_t>() { return U_CHAR(' '); }

// Whitespace is any of these:
// ' ' (0x20)space(SPC)
// '\t'(0x09)horizontal tab(TAB)
// '\n'(0x0a)newline(LF)
// '\v'(0x0b)vertical tab(VT)
// '\f'(0x0c)feed(FF)
// '\r'(0x0d)carriage return (CR)
template<typename CharType> constexpr bool Is_Whitespace(CharType ch)
{
#ifdef THYME_USE_STLPORT
    return ch == Get_Space<CharType>()
        || ch == Get_Tab<CharType>()
        || ch == Get_LF<CharType>()
        || ch == Get_VTab<CharType>()
        || ch == Get_FF<CharType>()
        || ch == Get_CR<CharType>();
#else
    return std::isspace(ch, std::locale());
#endif
}

template<typename CharType> constexpr bool Is_Null(CharType ch)
{
    return ch == Get_Null<CharType>();
}

template<typename CharType> constexpr bool Is_Null_Or_Whitespace(CharType ch)
{
    return Is_Null(ch) || Is_Whitespace(ch);
}

// clang-format on

enum StripOption
{
    STRIP_DEFAULT = 0,
    STRIP_REPLACE_WHITESPACE = BIT(0), // Stripping will replace all whitespace with space character.
    STRIP_LEADING_AND_TRAILING_ONLY = BIT(1), // Stripping within a sentence is prevented.
};
DEFINE_ENUMERATION_BITWISE_OPERATORS(StripOption);

// Strips leading, trailing and duplicate whitespace. Returns count of stripped characters.
// Before: "    Hello  there!   " After: "Hello there!"
// Compatible with UTF-8 and UTF-16.
template<typename CharType> int Strip_Whitespace(CharType *begin, CharType *end, StripOption option = STRIP_DEFAULT)
{
    captainslog_assert(end >= begin);
    captainslog_assert((end - begin) < 0x0fff);

    if (begin == end)
        return 0;

    constexpr CharType null_char = Get_Null<CharType>();
    CharType *reader = begin;
    CharType *reader_end = end;
    CharType *inserter = begin;

    const bool replace_whitespace = (option & STRIP_REPLACE_WHITESPACE) != 0;
    const bool leading_and_trailing_only = (option & STRIP_LEADING_AND_TRAILING_ONLY) != 0;
    const bool is_null_terminated = Is_Null<CharType>(*(reader_end - 1));

    // Increment reader to first non-whitespace character.
    for (; reader != reader_end && Is_Null_Or_Whitespace(*reader); ++reader) {
    }

    // Decrement reader_end to first non-whitespace character.
    for (; reader != reader_end && Is_Null_Or_Whitespace(*(reader_end - 1)); --reader_end) {
    }

    // The end was moved forward. If the original end was null terminated, then null will be put at new end.
    if (is_null_terminated) {
        *reader_end++ = null_char;
    }

    while (reader != reader_end) {
        CharType curr_char = *reader;
        CharType next_char = (++reader != reader_end) ? *reader : null_char;
        const bool curr_char_is_space = Is_Whitespace(curr_char);

        // If this and next character is any whitespace or null, then skip.
        if (!leading_and_trailing_only && curr_char_is_space && Is_Null_Or_Whitespace(next_char)) {
            continue;
        }

        // If this character is any whitespace, then turn it into a space character.
        if (replace_whitespace && curr_char_is_space) {
            curr_char = Get_Space<CharType>();
        }

        // Write out.
        *inserter++ = curr_char;
    }

    return (reader - inserter) + (end - reader_end);
}

// Strips leading, trailing and duplicate whitespace. Returns count of stripped characters.
// Before: "    Hello  there!   " After: "Hello there!"
// Compatible with UTF-8 and UTF-16.
template<typename StringType> int Strip_Whitespace(StringType &string, StripOption option = STRIP_DEFAULT)
{
    using CharType = typename StringType::value_type;

    if ((option & STRIP_LEADING_AND_TRAILING_ONLY) != 0) {
        // Run limited stripping mode. Requires full string length, including null terminator, to begin with.

        CharType *begin = string.Peek();
        CharType *end = string.Peek() + string.Get_Length() + 1;

        return Strip_Whitespace(begin, end, option);
    } else {
        // Run full stripping mode. Optimized for null terminated string.

        constexpr CharType null_char = Get_Null<CharType>();
        CharType *reader = string.Peek();
        CharType *inserter = string.Peek();

        const bool replace_whitespace = (option & STRIP_REPLACE_WHITESPACE) != 0;

        // Iterate to first non-whitespace character.
        for (; *reader != null_char && Is_Whitespace(*reader); ++reader) {
        }

        while (*reader != null_char) {
            CharType curr_char = *reader;
            CharType next_char = *++reader;
            const bool curr_char_is_space = Is_Whitespace(curr_char);

            // If this and next character is any whitespace or null, then skip.
            if (curr_char_is_space && Is_Null_Or_Whitespace(next_char)) {
                continue;
            }

            // If this character is any whitespace, then turn it into a space character.
            if (curr_char_is_space && replace_whitespace) {
                curr_char = Get_Space<CharType>();
            }

            // Write out.
            *inserter++ = curr_char;
        }

        // Close with null terminator always.
        *inserter = null_char;

        return (reader - inserter);
    }
}

} // namespace rts
