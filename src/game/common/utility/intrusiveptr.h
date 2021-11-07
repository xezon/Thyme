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

namespace rts
{
// #FEATURE Intrusive reference counted smart pointer.
// Works similar to std::shared_ptr<>, but does not hold reference counter by itself.
// Can be assigned a raw pointer at any time without worrying about colliding with another reference counter.
template<class Value> class intrusive_ptr
{
public:
    using element_type = Value;
    using value_type = Value;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using integer_type = typename Value::integer_type;

private:
    value_type *m_ptr;

public:
    intrusive_ptr() : m_ptr(nullptr) {}

    intrusive_ptr(value_type *ptr)
    {
        if (ptr)
            ptr->AddRef();
        m_ptr = ptr;
    }

    intrusive_ptr(const intrusive_ptr &other)
    {
        if (other.m_ptr)
            other.m_ptr->AddRef();
        m_ptr = other.m_ptr;
    }

    intrusive_ptr(intrusive_ptr &&other) noexcept
    {
        m_ptr = other.m_ptr;
        other.m_ptr = nullptr;
    }

    template<typename RelatedType> intrusive_ptr(const intrusive_ptr<RelatedType> &other)
    {
        if (other.m_ptr)
            other.m_ptr->AddRef();
        m_ptr = other.m_ptr;
    }

    ~intrusive_ptr()
    {
        if (m_ptr)
            m_ptr->Release();
    }

    intrusive_ptr &operator=(const intrusive_ptr &other)
    {
        if (this != &other) {
            if (other.m_ptr)
                other.m_ptr->AddRef();
            if (m_ptr)
                m_ptr->Release();
            m_ptr = other.m_ptr;
        }
        return *this;
    }

    intrusive_ptr &operator=(intrusive_ptr &&other)
    {
        if (this != &other) {
            if (m_ptr)
                m_ptr->Release();
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }
        return *this;
    }

    template<typename RelatedType> intrusive_ptr &operator=(const intrusive_ptr<RelatedType> &other)
    {
        if (other.m_ptr)
            other.m_ptr->AddRef();
        if (m_ptr)
            m_ptr->Release();
        m_ptr = other.m_ptr;
        return *this;
    }

    intrusive_ptr &operator=(value_type *ptr)
    {
        if (ptr)
            ptr->AddRef();
        if (m_ptr)
            m_ptr->Release();
        m_ptr = ptr;
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
    void reset() { intrusive_ptr().swap(*this); }

    // Reset smart pointer with new pointer.
    void reset(value_type *ptr)
    {
        if (ptr != m_ptr) {
            intrusive_ptr(ptr).swap(*this);
        }
    }

    // Swap smart pointer with other smart pointer.
    void swap(intrusive_ptr &other) { std::swap(m_ptr, other.m_ptr); }

    // Assign pointer without raising its reference count.
    // Used for example if the reference target already has a reference count on creation.
    void assign_without_add_ref(value_type *ptr)
    {
        reset();
        m_ptr = ptr;
    }

    // Release ownership of the pointer and do not change reference count.
    value_type *release()
    {
        value_type *ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

    // Get the count of how many times the pointer is referenced.
    integer_type use_count() const { return m_ptr->UseCount(); }
};

} // namespace rts
