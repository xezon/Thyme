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

#include "unichar.h"

namespace rts
{
template<typename CharType> CharType Get_Null();
template<typename CharType> bool Is_Space_Or_Tab(CharType ch);

template<> char Get_Null<char>()
{
    return '\0';
}

template<> unichar_t Get_Null<unichar_t>()
{
    return U_CHAR('\0');
}

template<> bool Is_Space_Or_Tab<char>(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\v';
}

template<> bool Is_Space_Or_Tab<unichar_t>(unichar_t ch)
{
    return ch == U_CHAR(' ') || ch == U_CHAR('\t') || ch == U_CHAR('\v');
}
} // namespace rts
