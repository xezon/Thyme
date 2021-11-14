/**
 * @file
 *
 * @author xezon
 *
 * @brief General purpose intrusive and extrusive smart pointer to wrap any type with (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "refcounter.h"
#include <algorithm>

namespace Thyme
{
namespace rts
{
// #FEATURE Non-intrusive reference counted smart pointer.
// Works similar to std::shared_ptr<>. Prefer using intrusive_ptr<> over nonintrusive_ptr_t<>.
// Provide a custom deleter when not using new & delete. Uses thread unsafe counter by default.
// If multiple threads are involved, then nonintrusive_atomic_counter can be used.
// Must not assign a raw pointer that has already been assigned to another nonintrusive_ptr_t<>.
template<class Value, typename Deleter = NewDeleter<Value>, typename Counter = nonintrusive_counter> class nonintrusive_ptr_t
{
public:
    using element_type = Value;
    using value_type = Value;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using counter_type = Counter;
    using integer_type = typename Counter::integer_type;

private:
    value_type *m_ptr;
    mutable counter_type *m_counter;

public:
    nonintrusive_ptr_t() : m_ptr(nullptr), m_counter(nullptr) {}

    nonintrusive_ptr_t(value_type *ptr)
    {
        m_ptr = ptr;
        m_counter = nullptr;
        if (m_ptr) {
            add_ref_for(m_counter);
        }
    }

    nonintrusive_ptr_t(const nonintrusive_ptr_t &other)
    {
        m_ptr = other.m_ptr;
        m_counter = other.m_counter;
        add_ref_for(m_counter);
    }

    nonintrusive_ptr_t(nonintrusive_ptr_t &&other) noexcept
    {
        m_ptr = other.m_ptr;
        m_counter = other.m_counter;
        other.m_ptr = nullptr;
        other.m_counter = nullptr;
    }

    template<typename RelatedType> nonintrusive_ptr_t(const nonintrusive_ptr_t<RelatedType, counter_type> &other)
    {
        m_ptr = other.m_ptr;
        m_counter = other.m_counter;
        add_ref_for(m_counter);
    }

    ~nonintrusive_ptr_t() { remove_ref_for(m_counter, m_ptr); }

    nonintrusive_ptr_t &operator=(const nonintrusive_ptr_t &other)
    {
        add_ref_for(other.m_counter);
        remove_ref_for(m_counter, m_ptr);
        m_ptr = other.m_ptr;
        m_counter = other.m_counter;
        return *this;
    }

    nonintrusive_ptr_t &operator=(nonintrusive_ptr_t &&other)
    {
        if (this != &other) {
            remove_ref_for(m_counter, m_ptr);
            m_ptr = other.m_ptr;
            m_counter = other.m_counter;
            other.m_ptr = nullptr;
            other.m_counter = nullptr;
        }
        return *this;
    }

    template<typename RelatedType> nonintrusive_ptr_t &operator=(const nonintrusive_ptr_t<RelatedType, counter_type> &other)
    {
        add_ref_for(other.m_counter);
        remove_ref_for(m_counter, m_ptr);
        m_ptr = other.m_ptr;
        m_counter = other.m_counter;
        return *this;
    }

    nonintrusive_ptr_t &operator=(value_type *ptr)
    {
        reset(ptr);
        return *this;
    }

    reference operator*() { return *m_ptr; }
    pointer operator->() { return m_ptr; }
    pointer get() { return m_ptr; }

    const_reference operator*() const { return *m_ptr; }
    const_pointer operator->() const { return m_ptr; }
    const_pointer get() const { return m_ptr; }

    operator bool() const { return m_ptr != nullptr; }

    // Reset smart pointer to null.
    void reset()
    {
        remove_ref_for(m_counter, m_ptr);
        m_ptr = nullptr;
        m_counter = nullptr;
    }

    // Reset smart pointer with new pointer.
    void reset(value_type *ptr)
    {
        if (ptr != m_ptr) {
            nonintrusive_ptr_t(ptr).swap(*this);
        }
    }

    // Swap smart pointer with other smart pointer.
    void swap(nonintrusive_ptr_t &other)
    {
        std::swap(m_ptr, other.m_ptr);
        std::swap(m_counter, other.m_counter);
    }

    // Get the count of how many times the pointer is referenced.
    integer_type use_count() const { m_counter->UseCount(); }

private:
    // #TODO Maybe allocate reference counter in game memory pool for better locality, if it makes any difference.

    static void add_ref_for(counter_type *&counter)
    {
        if (counter == nullptr) {
            counter = new Counter;
            counter->AddRef();
        } else {
            counter->AddRef();
        }
    }

    static void remove_ref_for(counter_type *&counter, value_type *&ptr)
    {
        if (counter != nullptr) {
            const integer_type ref = counter->Release();
            if (ref == 0) {
                Deleter::Delete_Instance(ptr);
                delete counter;
                ptr = nullptr;
                counter = nullptr;
            }
        }
    }
};

// Additional aliases for convenience.

template<typename Value, typename Deleter = NewDeleter<Value>>
using nonintrusive_ptr = nonintrusive_ptr_t<Value, Deleter, nonintrusive_counter>;

template<typename Value, typename Deleter = NewDeleter<Value>>
using nonintrusive_atomic_ptr = nonintrusive_ptr_t<Value, Deleter, nonintrusive_atomic_counter>;

} // namespace rts
} // namespace Thyme
