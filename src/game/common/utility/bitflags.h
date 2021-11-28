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

#ifdef THYME_USE_STLPORT
#include "sizedinteger.h"
#else
#include <type_traits>
#endif
#include <cstddef>

namespace rts
{

// clang-format off

template<typename ValueType> class bitflags
{
public:
    using Value = ValueType;
    using value_type = ValueType;

private:
#ifdef THYME_USE_STLPORT
    using underlying_type = typename unsigned_integer<value_type>::type;
#else
    using underlying_type = typename std::underlying_type<value_type>::type;
#endif

public:
    constexpr bitflags() noexcept : m_value(0) {}
    constexpr explicit bitflags(value_type value) noexcept : m_value(0) { set(value); }
    constexpr bitflags(const bitflags &other) noexcept : m_value(other.m_value) {}

    template<typename... Values> constexpr bitflags(Values... values) noexcept
    {
        for (value_type value : { values... }) {
            set(value);
        }
    }

    constexpr bitflags &operator=(bitflags other)          noexcept { m_value =  other.m_value; return *this; }
    constexpr bitflags &operator|=(bitflags other)         noexcept { m_value |= other.m_value; return *this; }
    constexpr bitflags &operator&=(bitflags other)         noexcept { m_value &= other.m_value; return *this; }
    constexpr bitflags &operator^=(bitflags other)         noexcept { m_value ^= other.m_value; return *this; }
    constexpr bitflags &operator<<=(std::size_t pos)       noexcept { m_value <<= pos; return *this; }
    constexpr bitflags &operator>>=(std::size_t pos)       noexcept { m_value >>= pos; return *this; }

    constexpr bool operator==(bitflags other)        const noexcept { return m_value == other.m_value; }
    constexpr bool operator!=(bitflags other)        const noexcept { return m_value != other.m_value; }
    constexpr operator bool()                        const noexcept { return m_value != underlying_type(0); }
    constexpr bitflags operator~()                   const noexcept { return bitflags(static_cast<value_type>(~m_value)); }
    constexpr bitflags operator|(bitflags other)     const noexcept { return bitflags(static_cast<value_type>(m_value | other.m_value)); }
    constexpr bitflags operator&(bitflags other)     const noexcept { return bitflags(static_cast<value_type>(m_value & other.m_value)); }
    constexpr bitflags operator^(bitflags other)     const noexcept { return bitflags(static_cast<value_type>(m_value ^ other.m_value)); }
    constexpr bitflags operator<<(std::size_t pos)   const noexcept { return bitflags(static_cast<value_type>(m_value << pos)); }
    constexpr bitflags operator>>(std::size_t pos)   const noexcept { return bitflags(static_cast<value_type>(m_value >> pos)); }

    constexpr void set(value_type value)                   noexcept { m_value |= static_cast<underlying_type>(value); }
    constexpr void set(bitflags flags)                     noexcept { m_value |= flags.m_value; }
    constexpr void reset(value_type value)                 noexcept { m_value &= ~static_cast<underlying_type>(value); }
    constexpr void reset(bitflags flags)                   noexcept { m_value &= ~flags.m_value; }
    constexpr void reset()                                 noexcept { m_value = underlying_type(0); }

    constexpr std::size_t size()                     const noexcept { return sizeof(m_value) * 8; }

    constexpr bool none()                            const noexcept { return (m_value == underlying_type(0)); }
    constexpr bool any()                             const noexcept { return (m_value != underlying_type(0)); }
    constexpr bool all()                             const noexcept { return (m_value == ~underlying_type(0)); }
    constexpr bool has(value_type value)             const noexcept { return ((m_value & static_cast<underlying_type>(value)) != underlying_type(0)); }
    constexpr bool has_only(value_type value)        const noexcept { return ((m_value & static_cast<underlying_type>(value)) == m_value); }
    constexpr bool has_any_of(bitflags flags)        const noexcept { return ((m_value & flags.m_value) != underlying_type(0)); }
    constexpr bool has_all_of(bitflags flags)        const noexcept { return ((m_value & flags.m_value) == flags.m_value); }

private:
    underlying_type m_value;
};

// clang-format on

} // namespace rts
