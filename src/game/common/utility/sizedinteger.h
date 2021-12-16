/**
 * @file
 *
 * @author xezon
 *
 * @brief Sized integer type
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "bittype.h"

namespace rts
{

// clang-format off

template<size_t Size> struct signed_integer_for_size;
template<size_t Size> struct unsigned_integer_for_size;

template<> struct signed_integer_for_size<1>{ using type = int8_t ; };
template<> struct signed_integer_for_size<2>{ using type = int16_t; };
template<> struct signed_integer_for_size<4>{ using type = int32_t; };
template<> struct signed_integer_for_size<8>{ using type = int64_t; };

template<> struct unsigned_integer_for_size<1>{ using type = uint8_t ; };
template<> struct unsigned_integer_for_size<2>{ using type = uint16_t; };
template<> struct unsigned_integer_for_size<4>{ using type = uint32_t; };
template<> struct unsigned_integer_for_size<8>{ using type = uint64_t; };

template<class T> struct signed_integer{ using type = typename signed_integer_for_size<sizeof(T)>::type; };
template<class T> struct unsigned_integer{ using type = typename unsigned_integer_for_size<sizeof(T)>::type; };

// clang-format off

}
