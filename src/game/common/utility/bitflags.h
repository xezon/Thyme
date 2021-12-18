/**
 * @file
 *
 * @author xezon
 *
 * @brief General purpose bit flags utility class (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "sizedinteger.h"
#include "utility/type_traits.h"
#include <cstddef>

namespace rts
{

template<typename ValueType, typename UnderlyingType = underlying_type_t<ValueType>> class bitflags
{
public:
    using Value = ValueType;
    using value_type = ValueType;
    using size_type = std::size_t;

private:
    using underlying_type = UnderlyingType;

public:
    constexpr bitflags() noexcept : m_value(0) {}

    constexpr bitflags(value_type value) noexcept : m_value(0) { set(value); }

    constexpr bitflags(const bitflags &other) noexcept : m_value(other.m_value) {}

    constexpr bitflags &operator=(bitflags other) noexcept
    {
        m_value = other.m_value;
        return *this;
    }
    constexpr bitflags &operator|=(bitflags other) noexcept
    {
        m_value |= other.m_value;
        return *this;
    }
    constexpr bitflags &operator&=(bitflags other) noexcept
    {
        m_value &= other.m_value;
        return *this;
    }
    constexpr bitflags &operator^=(bitflags other) noexcept
    {
        m_value ^= other.m_value;
        return *this;
    }
    constexpr bitflags &operator<<=(size_type pos) noexcept
    {
        m_value <<= pos;
        return *this;
    }
    constexpr bitflags &operator>>=(size_type pos) noexcept
    {
        m_value >>= pos;
        return *this;
    }

    constexpr bool operator==(bitflags other) const noexcept { return m_value == other.m_value; }

    constexpr bool operator!=(bitflags other) const noexcept { return m_value != other.m_value; }

    constexpr operator bool() const noexcept { return m_value != underlying_type(0); }

    constexpr bitflags operator~() const noexcept { return bitflags(static_cast<value_type>(~m_value)); }

    constexpr bitflags operator|(bitflags other) const noexcept
    {
        return bitflags(static_cast<value_type>(m_value | other.m_value));
    }

    constexpr bitflags operator&(bitflags other) const noexcept
    {
        return bitflags(static_cast<value_type>(m_value & other.m_value));
    }

    constexpr bitflags operator^(bitflags other) const noexcept
    {
        return bitflags(static_cast<value_type>(m_value ^ other.m_value));
    }

    constexpr bitflags operator<<(size_type pos) const noexcept { return bitflags(static_cast<value_type>(m_value << pos)); }

    constexpr bitflags operator>>(size_type pos) const noexcept { return bitflags(static_cast<value_type>(m_value >> pos)); }

    constexpr void set(value_type value) noexcept { m_value |= static_cast<underlying_type>(value); }

    constexpr void set(bitflags flags) noexcept { m_value |= flags.m_value; }

    constexpr void reset(value_type value) noexcept { m_value &= ~static_cast<underlying_type>(value); }

    constexpr void reset(bitflags flags) noexcept { m_value &= ~flags.m_value; }

    constexpr void reset() noexcept { m_value = underlying_type(0); }

    constexpr size_type size() const noexcept { return static_cast<size_type>(sizeof(m_value) * 8); }

    constexpr size_type count() const noexcept
    {
        size_type count = 0;
        for (size_type i = 0; i < size(); ++i) {
            value_type value = static_cast<value_type>(underlying_type(1) << underlying_type(i));
            if (has(value)) {
                ++count;
            }
        }
        return count;
    }

    constexpr bool none() const noexcept { return (m_value == underlying_type(0)); }

    constexpr bool any() const noexcept { return (m_value != underlying_type(0)); }

    constexpr bool all() const noexcept { return (m_value == ~underlying_type(0)); }

    constexpr bool has(value_type value) const noexcept
    {
        return ((m_value & static_cast<underlying_type>(value)) != underlying_type(0));
    }

    constexpr bool has_only(value_type value) const noexcept
    {
        return ((m_value & static_cast<underlying_type>(value)) == m_value);
    }

    constexpr bool has_any_of(bitflags flags) const noexcept { return ((m_value & flags.m_value) != underlying_type(0)); }

    constexpr bool has_all_of(bitflags flags) const noexcept { return ((m_value & flags.m_value) == flags.m_value); }

    constexpr bool get(value_type &value, size_type occurence = 0) const noexcept
    {
        size_type num = 0;
        for (size_type i = 0; i < count(); ++i) {
            if (has(static_cast<value_type>(i))) {
                if (occurence == num++) {
                    value = static_cast<value_type>(i);
                    return true;
                }
            }
        }
        return false;
    }

private:
    underlying_type m_value;
};

} // namespace rts
