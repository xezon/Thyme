/**
 * @file
 *
 * @author xezon
 *
 * @brief General purpose reference counter to inherit from (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "atomicop.h"
#include "bittype.h"
#include "captainslog.h"
#include "deleter.h"

// Functions follows naming convention of Windows referenced counted objects, such as DirectX, COM.

#if GAME_DEBUG
#ifndef REFCOUNTER_CHECK
#define REFCOUNTER_CHECK 1
#endif
#endif

#if !THYME_USE_STLPORT
#ifndef REFCOUNTER_USE_STD_ATOMIC
#define REFCOUNTER_USE_STD_ATOMIC 1
#endif
#endif

namespace Thyme
{
namespace rts
{
namespace detail
{
// #TODO Use sized integer here

// #FEATURE Helper function to check reference counter.
template<typename Integer> inline void Destructor_Ref_Check(Integer counter, Integer expected_counter = Integer{ 0 })
{
#if REFCOUNTER_CHECK
    if (static_cast<int>(counter) < static_cast<int>(expected_counter))
        captainslog_dbgassert(false,
            "REFCOUNTER_CHECK Deleting reference counted object more than once. Counter %d is not equal %d.",
            static_cast<int>(counter),
            static_cast<int>(expected_counter));

    if (static_cast<int>(counter) > static_cast<int>(expected_counter))
        captainslog_dbgassert(false,
            "REFCOUNTER_CHECK Deleting reference counted object without releasing all owners. Counter %d is not equal %d.",
            static_cast<int>(counter),
            static_cast<int>(expected_counter));
#endif
}

// #FEATURE Helper function to check reference counter.
template<typename Integer> inline void AddRef_Check(Integer counter, Integer min_counter = Integer{ 1 })
{
#if REFCOUNTER_CHECK
    captainslog_dbgassert(static_cast<int>(counter) >= static_cast<int>(min_counter),
        "REFCOUNTER_CHECK Unexpected reference add. Counter %d is smaller than %d.",
        static_cast<int>(counter),
        static_cast<int>(min_counter));
#endif
}

// #FEATURE Helper function to check reference counter.
template<typename Integer> inline void Release_Check(Integer counter, Integer min_counter = Integer{ 0 })
{
#if REFCOUNTER_CHECK
    captainslog_dbgassert(static_cast<int>(counter) >= static_cast<int>(min_counter),
        "REFCOUNTER_CHECK Unexpected reference removal. Counter %d is smaller than %d.",
        static_cast<int>(counter),
        static_cast<int>(min_counter));
#endif
}

template<typename Integer> inline Integer AddRef(Integer &counter)
{
    ++counter;
    AddRef_Check(counter);
    return counter;
}

template<typename Integer, typename Instance, typename Deleter>
inline Integer Release(Integer &counter, const Instance *instance)
{
    if (--counter == Integer{ 0 }) {
        Deleter::Delete_Instance(const_cast<Instance *>(instance));
    } else {
        Release_Check(counter);
    }
    return counter;
}

template<typename Integer> inline Integer Release(Integer &counter)
{
    --counter;
    Release_Check(counter);
    return counter;
}

template<typename Integer> inline Integer AddRef_Atomic(volatile Integer &counter)
{
    const Integer new_count = Atomic_Increment(&counter);
    AddRef_Check(new_count);
    return new_count;
}

template<typename Integer, typename Instance, typename Deleter>
inline Integer Release_Atomic(volatile Integer &counter, const Instance *instance)
{
    const Integer new_counter = Atomic_Decrement(&counter);
    if (new_counter == Integer{ 0 }) {
        Deleter::Delete_Instance(const_cast<Instance *>(instance));
    } else {
        Release_Check(new_counter);
    }
    return new_counter;
}

template<typename Integer> inline Integer Release_Atomic(volatile Integer &counter)
{
    const Integer new_counter = Atomic_Decrement(&counter);
    Release_Check(new_counter);
    return new_counter;
}

} // namespace detail

// #FEATURE Intrusive atomic counter for multi threaded use. Can be used with: intrusive_ptr<>.
template<typename Integer, typename Derived, typename Deleter = NewDeleter<Derived>> class intrusive_atomic_counter_i
{
public:
    using integer_type = Integer;
    using derived_type = Derived;
    using deleter_type = Deleter;

    intrusive_atomic_counter_i() = default;
    intrusive_atomic_counter_i(const intrusive_atomic_counter_i &) {}
    intrusive_atomic_counter_i &operator=(const intrusive_atomic_counter_i &) { return *this; }

#if REFCOUNTER_CHECK
    // Virtual Destructor to make sure this is called always on any deletion attempt.
    virtual ~intrusive_atomic_counter_i() { detail::Destructor_Ref_Check(m_counter); }
#else
    // Deleted destructor. This avoids forcing a virtual table in any derived class.
    // Not calling destructor is legal and will not break anything, because the destructor does not do anything anyway.
    ~intrusive_counter_i() = delete;
#endif

    integer_type AddRef() const { return detail::AddRef_Atomic<integer_type>(m_counter); }
    integer_type Release() const
    {
        return detail::Release_Atomic<integer_type, derived_type, deleter_type>(
            m_counter, static_cast<const derived_type *>(this));
    }
    integer_type UseCount() const
    {
        captainslog_dbgassert(false, "Use count cannot be used in multi threaded context");
        return integer_type{ -1 };
    }

private:
    mutable volatile integer_type m_counter = { 0 };
};

// #FEATURE Non-intrusive atomic counter for multi threaded use. Can be used with: nonintrusive_ptr<>.
template<typename Integer> class nonintrusive_atomic_counter_i
{
public:
    using integer_type = Integer;

    nonintrusive_atomic_counter_i() = default;
    nonintrusive_atomic_counter_i(const nonintrusive_atomic_counter_i &) = delete;
    nonintrusive_atomic_counter_i &operator=(const nonintrusive_atomic_counter_i &) = delete;
    ~nonintrusive_atomic_counter_i() { detail::Destructor_Ref_Check(m_counter); }

    integer_type AddRef() const { return detail::AddRef_Atomic<integer_type>(m_counter); }
    integer_type Release() const { return detail::Release_Atomic<integer_type>(m_counter); }
    integer_type UseCount() const
    {
        captainslog_dbgassert(false, "Use count cannot be used in multi threaded context");
        return integer_type{ -1 };
    }

private:
    mutable volatile integer_type m_counter = { 0 };
};

// #FEATURE Intrusive counter for single threaded use. Can be used with intrusive_ptr<>.
// #TODO Add debug feature to verify counter is not touched from 2 different threads.
template<typename Integer, typename Derived, typename Deleter = NewDeleter<Derived>> class intrusive_counter_i
{
public:
    using integer_type = Integer;
    using derived_type = Derived;
    using deleter_type = Deleter;

    intrusive_counter_i() = default;
    intrusive_counter_i(const intrusive_counter_i &) {}
    intrusive_counter_i &operator=(const intrusive_counter_i &) { return *this; }

#if REFCOUNTER_CHECK
    // Virtual Destructor to make sure this is called always on any deletion attempt.
    virtual ~intrusive_counter_i() { detail::Destructor_Ref_Check(m_counter); }
#else
    // Deleted destructor. This avoids forcing a virtual table in any derived class.
    // Not calling destructor is legal and will not break anything, because the destructor does not do anything anyway.
    ~intrusive_counter_i() = delete;
#endif

    integer_type AddRef() const { return detail::AddRef<integer_type>(m_counter); }
    integer_type Release() const
    {
        return detail::Release<integer_type, derived_type, deleter_type>(m_counter, static_cast<const derived_type *>(this));
    }
    integer_type UseCount() const { return m_counter; }

private:
    mutable integer_type m_counter = { 0 };
};

// #FEATURE Non-intrusive counter for single threaded use. Can be used with nonintrusive_ptr<>.
// #TODO Add debug feature to verify counter is not touched from 2 different threads.
template<typename Integer> class nonintrusive_counter_i
{
public:
    using integer_type = Integer;

    nonintrusive_counter_i() = default;
    nonintrusive_counter_i(const nonintrusive_counter_i &) = delete;
    nonintrusive_counter_i &operator=(const nonintrusive_counter_i &) = delete;
    ~nonintrusive_counter_i() { detail::Destructor_Ref_Check(m_counter); }

    integer_type AddRef() const { return detail::AddRef<integer_type>(m_counter); }
    integer_type Release() const { return detail::Release<integer_type>(m_counter); }
    integer_type UseCount() const { return m_counter; }

private:
    mutable integer_type m_counter = { 0 };
};

// Additional aliases for convenience.

using RefCounterInteger = AtomicType32;

template<typename Derived, typename Deleter = NewDeleter<Derived>>
using intrusive_atomic_counter = intrusive_atomic_counter_i<RefCounterInteger, Derived, Deleter>;
template<typename Derived, typename Deleter = NewDeleter<Derived>>
using intrusive_counter = intrusive_counter_i<RefCounterInteger, Derived, Deleter>;

using nonintrusive_atomic_counter = nonintrusive_atomic_counter_i<RefCounterInteger>;
using nonintrusive_counter = nonintrusive_counter_i<RefCounterInteger>;

} // namespace rts
} // namespace Thyme
