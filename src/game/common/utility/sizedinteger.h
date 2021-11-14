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

template<> struct signed_integer_for_size<1>{ typedef int8_t  type; };
template<> struct signed_integer_for_size<2>{ typedef int16_t type; };
template<> struct signed_integer_for_size<4>{ typedef int32_t type; };
template<> struct signed_integer_for_size<8>{ typedef int64_t type; };

template<> struct unsigned_integer_for_size<1>{ typedef uint8_t  type; };
template<> struct unsigned_integer_for_size<2>{ typedef uint16_t type; };
template<> struct unsigned_integer_for_size<4>{ typedef uint32_t type; };
template<> struct unsigned_integer_for_size<8>{ typedef uint64_t type; };

template<class T> struct signed_integer{ typedef typename signed_integer_for_size<sizeof(T)>::type type; };
template<class T> struct unsigned_integer{ typedef typename unsigned_integer_for_size<sizeof(T)>::type type; };

// clang-format off

}
