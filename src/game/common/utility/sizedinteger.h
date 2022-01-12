/**
 * @file
 *
 * @author xezon
 *
 * @brief Sized integer type. (Thyme Feature)
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

// #TODO Remove this file and use "typesize.h" when available

namespace rts
{

// clang-format off

template<size_t Size> struct SignedIntegerForSize;
template<size_t Size> struct UnsignedIntegerForSize;

template<> struct SignedIntegerForSize<1>{ using type = int8_t; };
template<> struct SignedIntegerForSize<2>{ using type = int16_t; };
template<> struct SignedIntegerForSize<4>{ using type = int32_t; };
template<> struct SignedIntegerForSize<8>{ using type = int64_t; };

template<> struct UnsignedIntegerForSize<1>{ using type = uint8_t; };
template<> struct UnsignedIntegerForSize<2>{ using type = uint16_t; };
template<> struct UnsignedIntegerForSize<4>{ using type = uint32_t; };
template<> struct UnsignedIntegerForSize<8>{ using type = uint64_t; };

template<class T> struct SignedInteger{ using type = typename SignedIntegerForSize<sizeof(T)>::type; };
template<class T> struct UnsignedInteger{ using type = typename UnsignedIntegerForSize<sizeof(T)>::type; };

template<class T> using SignedIntegerT = typename SignedInteger<T>::type;
template<class T> using UnsignedIntegerT = typename UnsignedInteger<T>::type;

// clang-format off

} // namespace rts
