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

#include "stringutil.h"

namespace rts
{
namespace detail
{
const escaped_char_alias<char> s_escapedCharactersA[8] = {
    { get_char<char>('\0'), get_char<char>('\\'), get_char<char>('0') },
    { get_char<char>('\a'), get_char<char>('\\'), get_char<char>('a') },
    { get_char<char>('\b'), get_char<char>('\\'), get_char<char>('b') },
    { get_char<char>('\t'), get_char<char>('\\'), get_char<char>('t') },
    { get_char<char>('\n'), get_char<char>('\\'), get_char<char>('n') },
    { get_char<char>('\v'), get_char<char>('\\'), get_char<char>('v') },
    { get_char<char>('\f'), get_char<char>('\\'), get_char<char>('f') },
    { get_char<char>('\r'), get_char<char>('\\'), get_char<char>('r') },
};

const escaped_char_alias<unichar_t> s_escapedCharactersU[8] = {
    { get_char<unichar_t>('\0'), get_char<unichar_t>('\\'), get_char<unichar_t>('0') },
    { get_char<unichar_t>('\a'), get_char<unichar_t>('\\'), get_char<unichar_t>('a') },
    { get_char<unichar_t>('\b'), get_char<unichar_t>('\\'), get_char<unichar_t>('b') },
    { get_char<unichar_t>('\t'), get_char<unichar_t>('\\'), get_char<unichar_t>('t') },
    { get_char<unichar_t>('\n'), get_char<unichar_t>('\\'), get_char<unichar_t>('n') },
    { get_char<unichar_t>('\v'), get_char<unichar_t>('\\'), get_char<unichar_t>('v') },
    { get_char<unichar_t>('\f'), get_char<unichar_t>('\\'), get_char<unichar_t>('f') },
    { get_char<unichar_t>('\r'), get_char<unichar_t>('\\'), get_char<unichar_t>('r') },
};
} // namespace detail
} // namespace rts
