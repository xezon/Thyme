/**
 * @file
 *
 * @author xezon
 *
 * @brief Enumerator utility class (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifdef THYME_USE_STLPORT
#include "sizedinteger.h"
#else
#include <type_traits>
#endif
#include <cstddef>

namespace rts
{

// clang-format off

template<typename ValueType> class enumerator
{
public:
    using Value = ValueType;
    using value_type = ValueType;
#ifdef THYME_USE_STLPORT
    using underlying_type = typename unsigned_integer<value_type>::type;
#else
    using underlying_type = typename std::underlying_type<value_type>::type;
#endif

public:
    constexpr enumerator() = default;

    constexpr enumerator(value_type value) : m_value(static_cast<underlying_type>(value)) {}
    constexpr enumerator(const enumerator &other) : m_value(other.m_value) {}

    constexpr enumerator &operator= (enumerator other) { m_value =  other.m_value; return *this; }
    constexpr enumerator &operator+=(enumerator other) { m_value += other.m_value; return *this; }
    constexpr enumerator &operator-=(enumerator other) { m_value -= other.m_value; return *this; }
    constexpr enumerator &operator*=(enumerator other) { m_value *= other.m_value; return *this; }
    constexpr enumerator &operator/=(enumerator other) { m_value /= other.m_value; return *this; }
    constexpr enumerator &operator%=(enumerator other) { m_value %= other.m_value; return *this; }
    constexpr enumerator &operator++()                 { ++m_value; return *this; }
    constexpr enumerator &operator--()                 { --m_value; return *this; }

    constexpr enumerator operator+(enumerator other)   { return enumerator(static_cast<value_type>(m_value + other.m_value)); }
    constexpr enumerator operator-(enumerator other)   { return enumerator(static_cast<value_type>(m_value - other.m_value)); }
    constexpr enumerator operator*(enumerator other)   { return enumerator(static_cast<value_type>(m_value * other.m_value)); }
    constexpr enumerator operator/(enumerator other)   { return enumerator(static_cast<value_type>(m_value / other.m_value)); }
    constexpr enumerator operator%(enumerator other)   { return enumerator(static_cast<value_type>(m_value % other.m_value)); }
    constexpr enumerator operator++(int)               { return enumerator(static_cast<value_type>(m_value++)); }
    constexpr enumerator operator--(int)               { return enumerator(static_cast<value_type>(m_value--)); }

    constexpr bool operator==(enumerator other)  const { return m_value == other.m_value; }
    constexpr bool operator!=(enumerator other)  const { return m_value != other.m_value; }
    constexpr bool operator< (enumerator other)  const { return m_value <  other.m_value; }
    constexpr bool operator> (enumerator other)  const { return m_value >  other.m_value; }
    constexpr bool operator<=(enumerator other)  const { return m_value <= other.m_value; }
    constexpr bool operator>=(enumerator other)  const { return m_value >= other.m_value; }
    constexpr operator bool()                    const { return m_value != underlying_type(0); }

    constexpr void reset()                             { m_value = underlying_type(0); }

    constexpr value_type value()                 const { return static_cast<value_type>(m_value); }

private:
    underlying_type m_value = 0;
};

// clang-format on

} // namespace rts
