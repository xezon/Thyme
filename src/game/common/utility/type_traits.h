/**
 * @file
 *
 * @author xezon
 *
 * @brief Type traits, equivalent to std type traits (c++11, 14) (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

namespace rts
{

template<typename T> struct remove_cv
{
    using type = T;
};
template<typename T> struct remove_cv<const T>
{
    using type = T;
};
template<typename T> struct remove_cv<volatile T>
{
    using type = T;
};
template<typename T> struct remove_cv<const volatile T>
{
    using type = T;
};
template<typename T> struct remove_const
{
    using type = T;
};
template<typename T> struct remove_const<const T>
{
    using type = T;
};
template<typename T> struct remove_volatile
{
    using type = T;
};
template<typename T> struct remove_volatile<volatile T>
{
    using type = T;
};
template<typename T> struct remove_reference
{
    using type = T;
};
template<typename T> struct remove_reference<T &>
{
    using type = T;
};
template<typename T> struct remove_reference<T &&>
{
    using type = T;
};

template<typename T> using remove_cv_t = typename remove_cv<T>::type;
template<typename T> using remove_const_t = typename remove_const<T>::type;
template<typename T> using remove_volatile_t = typename remove_volatile<T>::type;
template<typename T> using remove_reference_t = typename remove_reference<T>::type;

} // namespace rts
