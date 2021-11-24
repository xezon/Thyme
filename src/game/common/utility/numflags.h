/**
 * @file
 *
 * @author xezon
 *
 * @brief General purpose number flags utility class (Thyme Feature)
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
#include <cstddef>

namespace rts
{

// clang-format off

namespace detail
{
template<typename IntegerType, std::size_t Bytes> struct BitBucketLshStruct;
template<typename IntegerType, std::size_t Bytes> struct BitBucketMaskStruct;

template<typename IntegerType> struct BitBucketLshStruct<IntegerType, 1>{ static constexpr IntegerType Get() { return IntegerType(3); } };
template<typename IntegerType> struct BitBucketLshStruct<IntegerType, 2>{ static constexpr IntegerType Get() { return IntegerType(4); } };
template<typename IntegerType> struct BitBucketLshStruct<IntegerType, 4>{ static constexpr IntegerType Get() { return IntegerType(5); } };
template<typename IntegerType> struct BitBucketLshStruct<IntegerType, 8>{ static constexpr IntegerType Get() { return IntegerType(6); } };

template<typename IntegerType> struct BitBucketMaskStruct<IntegerType, 1>{ static constexpr IntegerType Get() { return IntegerType(0xfff); } };
template<typename IntegerType> struct BitBucketMaskStruct<IntegerType, 2>{ static constexpr IntegerType Get() { return IntegerType(0xffff); } };
template<typename IntegerType> struct BitBucketMaskStruct<IntegerType, 4>{ static constexpr IntegerType Get() { return IntegerType(0xfffff); } };
template<typename IntegerType> struct BitBucketMaskStruct<IntegerType, 8>{ static constexpr IntegerType Get() { return IntegerType(0xffffff); } };

template<typename IntegerType, std::size_t Bytes> constexpr IntegerType Bit_Bucket_Lsh() { return BitBucketLshStruct<IntegerType, Bytes>::Get(); }
template<typename IntegerType, std::size_t Bytes> constexpr IntegerType Bit_Bucket_Mask() { return BitBucketMaskStruct<IntegerType, Bytes>::Get(); }
}

// clang-format on

template<typename ValueType, std::size_t Bits> class numflags
{
    template<typename, std::size_t> friend class numflags;
    using storage_type = uint32_t;

public:
    using Value = ValueType;
    using value_type = ValueType;

public:
    constexpr numflags() { reset(); }

    constexpr explicit numflags(value_type value)
    {
        reset();
        set(value);
    }

    constexpr numflags(const numflags &other) noexcept { copy(m_values, other.m_values, bucket_size()); }

    template<std::size_t OtherBits> constexpr numflags(const numflags<value_type, OtherBits> &other) noexcept
    {
        const std::size_t shared_bucket_size = std::min(bucket_size(), other.bucket_size());
        copy(m_values, other.m_values, shared_bucket_size);
        zero(m_values + shared_bucket_size, bucket_size() - shared_bucket_size);
    }

    template<typename... Values> constexpr numflags(Values... values) noexcept
    {
        reset();
        for (value_type value : { values... }) {
            set(value);
        }
    }

    constexpr numflags &operator=(const numflags &other) noexcept
    {
        copy(m_values, other.m_values, bucket_size());
        return *this;
    }

    template<std::size_t OtherBits> constexpr numflags &operator=(const numflags<value_type, OtherBits> &other) noexcept
    {
        const std::size_t shared_bucket_size = std::min(bucket_size(), other.bucket_size());
        copy(m_values, other.m_values, shared_bucket_size);
        return *this;
    }

    constexpr numflags &operator|=(const numflags &other) noexcept
    {
        for (std::size_t i = 0; i < bucket_size(); ++i) {
            m_values[i] |= other.m_values[i];
        }
        return *this;
    }

    template<std::size_t OtherBits> constexpr numflags &operator|=(const numflags<value_type, OtherBits> &other) noexcept
    {
        const std::size_t shared_bucket_size = std::min(bucket_size(), other.bucket_size());
        for (std::size_t i = 0; i < shared_bucket_size; ++i) {
            m_values[i] |= other.m_values[i];
        }
        return *this;
    }

    constexpr numflags &operator&=(const numflags &other) noexcept
    {
        for (std::size_t i = 0; i < bucket_size(); ++i) {
            m_values[i] &= other.m_values[i];
        }
        return *this;
    }

    template<std::size_t OtherBits> constexpr numflags &operator&=(const numflags<value_type, OtherBits> &other) noexcept
    {
        const std::size_t shared_bucket_size = std::min(bucket_size(), other.bucket_size());
        for (std::size_t i = 0; i < shared_bucket_size; ++i) {
            m_values[i] &= other.m_values[i];
        }
        return *this;
    }

    constexpr numflags &operator^=(const numflags &other) noexcept
    {
        for (std::size_t i = 0; i < bucket_size(); ++i) {
            m_values[i] ^= other.m_values[i];
        }
        return *this;
    }

    template<std::size_t OtherBits> constexpr numflags &operator^=(const numflags<value_type, OtherBits> &other) noexcept
    {
        const std::size_t shared_bucket_size = std::min(bucket_size(), other.bucket_size());
        for (std::size_t i = 0; i < shared_bucket_size; ++i) {
            m_values[i] ^= other.m_values[i];
        }
        return *this;
    }

    // constexpr numflags &operator<<=(std::size_t pos); // #TODO implement
    // constexpr numflags &operator>>=(std::size_t pos);

    constexpr bool operator==(const numflags &other) const noexcept
    {
        return 0 == compare(m_values, other.m_values, sizeof(m_values));
    }

    template<std::size_t OtherBits> constexpr bool operator==(const numflags<value_type, OtherBits> &other) noexcept
    {
        const std::size_t shared_bucket_bytes = std::min(bucket_bytes(), other.bucket_bytes());
        return 0 == compare(m_values, other.m_values, shared_bucket_bytes);
    }

    constexpr bool operator!=(const numflags &other) const noexcept { return !operator==(other); }

    template<std::size_t OtherBits> constexpr bool operator!=(const numflags<value_type, OtherBits> &other) noexcept
    {
        return !operator==(other);
    }

    constexpr operator bool() const noexcept { return any(); }

    constexpr numflags operator~() const noexcept
    {
        numflags inst;
        for (std::size_t i = 0; i < bucket_size(); ++i) {
            inst.m_values[i] = ~m_values[i];
        }
        return inst;
    }

    constexpr numflags operator|(const numflags &other) const noexcept
    {
        numflags inst(*this);
        inst |= other;
        return inst;
    }

    template<std::size_t OtherBits> constexpr numflags operator|(const numflags<value_type, OtherBits> &other) noexcept
    {
        numflags inst(*this);
        inst |= other;
        return inst;
    }

    constexpr numflags operator&(const numflags &other) const noexcept
    {
        numflags inst(*this);
        inst &= other;
        return inst;
    }

    template<std::size_t OtherBits> constexpr numflags operator&(const numflags<value_type, OtherBits> &other) noexcept
    {
        numflags inst(*this);
        inst &= other;
        return inst;
    }

    constexpr numflags operator^(const numflags &other) const noexcept
    {
        numflags inst(*this);
        inst ^= other;
        return inst;
    }

    template<std::size_t OtherBits> constexpr numflags operator^(const numflags<value_type, OtherBits> &other) noexcept
    {
        numflags inst(*this);
        inst ^= other;
        return inst;
    }

    // constexpr numflags operator<<(std::size pos) const; // #TODO implement
    // constexpr numflags operator>>(std::size pos) const;

    constexpr void set(value_type value) { access(value) |= bit(value); }

    constexpr void set(const numflags &flags) noexcept
    {
        for (std::size_t i = 0; i < bucket_size(); ++i) {
            m_values[i] |= flags.m_values[i];
        }
    }
    constexpr void reset(value_type value) { access(value) &= ~bit(value); }

    constexpr void reset(const numflags &flags) noexcept
    {
        for (std::size_t i = 0; i < bucket_size(); ++i) {
            m_values[i] &= ~flags.m_values[i];
        }
    }

    constexpr void reset() noexcept { zero(m_values, bucket_size()); }

    constexpr std::size_t size() const noexcept { return sizeof(m_values) * 8; }

    constexpr bool none() const noexcept
    {
        storage_type bits = 0;
        for (const storage_type value : m_values) {
            bits |= value;
        }
        return bits == storage_type(0);
    }

    constexpr bool any() const noexcept { return !none(); }

    constexpr bool all() const noexcept
    {
        storage_type bits = ~storage_type(0);
        for (const storage_type value : m_values) {
            bits &= value;
        }
        return bits == ~storage_type(0);
    }

    constexpr bool has(value_type value) const { return ((access(value) & bit(value)) != storage_type(0)); }

    constexpr bool has_only(value_type value) const
    {
        const std::size_t value_bucket = bucket(value);

        if ((m_values[value_bucket] & bit(value)) != m_values[value_bucket]) {
            return false;
        }
        storage_type other_bits = 0;
        std::size_t i = 0;

        for (; i < value_bucket; ++i) {
            other_bits |= m_values[i];
        }
        for (++i; i < bucket_size(); ++i) {
            other_bits |= m_values[i];
        }
        return (other_bits == storage_type(0));
    }

    constexpr bool has_any_of(numflags flags) const noexcept
    {
        bool test = false;
        for (std::size_t i = 0; i < bucket_size(); ++i) {
            test |= ((m_values[i] & flags.m_values[i]) != storage_type(0));
        }
        return test;
    }

    constexpr bool has_all_of(numflags flags) const noexcept
    {
        bool test = true;
        for (std::size_t i = 0; i < bucket_size(); ++i) {
            test &= ((m_values[i] & flags.m_values[i]) == flags.m_values[i]);
        }
        return test;
    }

private:
    static constexpr void copy(storage_type *dst, const storage_type *src, std::size_t count)
    {
        while (count-- > 0) {
            dst[count] = src[count];
        }
    }

    static constexpr void zero(storage_type *dst, std::size_t count)
    {
        while (count-- > 0) {
            dst[count] = storage_type(0);
        }
    }

    static constexpr int compare(const storage_type *left, const storage_type *right, std::size_t count)
    {
        while (count-- > 0) {
            if (*left++ != *right++)
                return left[-1] < right[-1] ? -1 : 1;
        }
        return 0;
    }

    static constexpr std::size_t bucket_size() noexcept { return sizeof(m_values) / sizeof(storage_type); }

    static constexpr std::size_t bucket_bytes() noexcept { return sizeof(m_values); }

    static constexpr storage_type bit(value_type value) noexcept
    {
        // Mask value to max storage bit count and shift to single storage bit. Example with 1 byte storage:
        // value   0101 1111 = 95
        // masked  0000 0111 = 7
        // shifted 1000 0000 = 128
        storage_type masked =
            static_cast<storage_type>(value) & detail::Bit_Bucket_Mask<storage_type, sizeof(storage_type)>();
        storage_type shifted = 1 << masked;
        return shifted;
    }

    template<typename IntegerType> static constexpr std::size_t bucket(IntegerType value)
    {
        // #TODO assert bucket overflow

        // Get bucket index with given value. Example with 1 byte storage:
        // value 0101 1111 = 95
        // index 0000 1011 = 11
        std::size_t index = static_cast<std::size_t>(value) >> detail::Bit_Bucket_Lsh<std::size_t, sizeof(storage_type)>();
        return index;
    }

    constexpr storage_type &access(value_type value) { return m_values[bucket(value)]; }

    constexpr storage_type access(value_type value) const { return m_values[bucket(value)]; }

private:
    storage_type m_values[1 + (bucket(Bits))];
};

} // namespace rts
