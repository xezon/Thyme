/**
 * @file
 *
 * @author xezon
 *
 * @brief General purpose reference counter to inherit from. (Thyme Feature)
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
#include "sizedinteger.h"

// Functions follows naming convention of Windows referenced counted objects, such as DirectX, COM.

#if GAME_DEBUG
#ifndef REFCOUNTER_CHECK
#define REFCOUNTER_CHECK 1
#endif
#endif

#if !THYME_USE_STLPORT
#ifndef REFCOUNTER_USE_STD_ATOMIC
#define REFCOUNTER_USE_STD_ATOMIC 1 // #TODO implement relevant functionality
#endif
#endif

namespace rts
{
namespace detail
{

template<typename Integer> inline void Destructor_Ref_Check(Integer counter, Integer expected_counter = Integer{ 0 })
{
#if REFCOUNTER_CHECK
    using SignedInt = SignedInteger<Integer>::type;
    if (static_cast<SignedInt>(counter) < static_cast<SignedInt>(expected_counter))
        captainslog_dbgassert(false,
            "REFCOUNTER_CHECK Deleting reference counted object more than once. Counter %d is not equal %d.",
            static_cast<SignedInt>(counter),
            static_cast<SignedInt>(expected_counter));

    if (static_cast<SignedInt>(counter) > static_cast<SignedInt>(expected_counter))
        captainslog_dbgassert(false,
            "REFCOUNTER_CHECK Deleting reference counted object without releasing all owners. Counter %d is not equal %d.",
            static_cast<SignedInt>(counter),
            static_cast<SignedInt>(expected_counter));
#endif
}

template<typename Integer> inline void AddRef_Check(Integer counter, Integer min_counter = Integer{ 1 })
{
#if REFCOUNTER_CHECK
    using SignedInt = SignedInteger<Integer>::type;
    captainslog_dbgassert(static_cast<SignedInt>(counter) >= static_cast<SignedInt>(min_counter),
        "REFCOUNTER_CHECK Unexpected reference add. Counter %d is smaller than %d.",
        static_cast<SignedInt>(counter),
        static_cast<SignedInt>(min_counter));
#endif
}

template<typename Integer> inline void Release_Check(Integer counter, Integer min_counter = Integer{ 0 })
{
#if REFCOUNTER_CHECK
    using SignedInt = SignedInteger<Integer>::type;
    captainslog_dbgassert(static_cast<SignedInt>(counter) >= static_cast<SignedInt>(min_counter),
        "REFCOUNTER_CHECK Unexpected reference removal. Counter %d is smaller than %d.",
        static_cast<SignedInt>(counter),
        static_cast<SignedInt>(min_counter));
#endif
}

template<typename Integer> inline Integer AddRef(Integer &counter)
{
    ++counter;
    AddRef_Check(counter);
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

template<typename Integer> inline Integer Release_Atomic(volatile Integer &counter)
{
    const Integer new_counter = Atomic_Decrement(&counter);
    Release_Check(new_counter);
    return new_counter;
}

} // namespace detail

// #FEATURE Intrusive atomic counter for multi threaded use. Can be used with: intrusive_ptr<>.
template<typename Integer, typename Derived, typename Deleter = NewDeleter<Derived>> class atomic_intrusive_counter_i
{
public:
    using integer_type = Integer;
    using derived_type = Derived;
    using deleter_type = Deleter;

    atomic_intrusive_counter_i() = default;
    atomic_intrusive_counter_i(const atomic_intrusive_counter_i &) {}
    atomic_intrusive_counter_i &operator=(const atomic_intrusive_counter_i &) { return *this; }

#if REFCOUNTER_CHECK
    // Virtual Destructor to make sure this is called always on any deletion attempt.
    virtual ~atomic_intrusive_counter_i() { detail::Destructor_Ref_Check(m_counter); }
#else
    // Non virtual destructor to avoid forcing virtual table on derived type.
    // Destructor could also be deleted in this class, but we keep it for good measure.
    ~atomic_intrusive_counter_i() {}
#endif

    integer_type AddRef() const { return detail::AddRef_Atomic<integer_type>(m_counter); }
    integer_type Release() const
    {
        const integer_type new_counter = Atomic_Decrement(&m_counter);
        if (new_counter == integer_type{ 0 }) {
#if !REFCOUNTER_CHECK
            this->~atomic_intrusive_counter_i();
#endif
            Invoke_Deleter<deleter_type>(const_cast<derived_type *>(static_cast<const derived_type *>(this)));
        } else {
            detail::Release_Check(new_counter);
        }
        return new_counter;
    }
    integer_type UseCount() const
    {
        captainslog_dbgassert(false, "Use count cannot be used in multi threaded context");
        return integer_type{ -1 };
    }

private:
    mutable volatile integer_type m_counter = integer_type{ 0 };
};

// #FEATURE Non-intrusive atomic counter for multi threaded use. Can be used with: shared_ptr_t<>.
template<typename Integer> class atomic_shared_counter_i
{
public:
    using integer_type = Integer;

    atomic_shared_counter_i() = default;
    atomic_shared_counter_i(const atomic_shared_counter_i &) = delete;
    atomic_shared_counter_i &operator=(const atomic_shared_counter_i &) = delete;
    ~atomic_shared_counter_i() { detail::Destructor_Ref_Check(m_counter); }

    integer_type AddRef() const { return detail::AddRef_Atomic<integer_type>(m_counter); }
    integer_type Release() const { return detail::Release_Atomic<integer_type>(m_counter); }
    integer_type UseCount() const
    {
        captainslog_dbgassert(false, "Use count cannot be used in multi threaded context");
        return integer_type{ -1 };
    }

private:
    mutable volatile integer_type m_counter = integer_type{ 0 };
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
    // Non virtual destructor to avoid forcing virtual table on derived type.
    // Destructor could also be deleted in this class, but we keep it for good measure.
    ~intrusive_counter_i() {}
#endif

    integer_type AddRef() const { return detail::AddRef<integer_type>(m_counter); }
    integer_type Release() const
    {
        integer_type counter = --m_counter;
        if (counter == integer_type{ 0 }) {
#if !REFCOUNTER_CHECK
            this->~intrusive_counter_i();
#endif
            Invoke_Deleter<deleter_type>(const_cast<derived_type *>(static_cast<const derived_type *>(this)));
        } else {
            detail::Release_Check(counter);
        }
        return counter;
    }
    integer_type UseCount() const { return m_counter; }

private:
    mutable integer_type m_counter = integer_type{ 0 };
};

// #FEATURE Non-intrusive counter for single threaded use. Can be used with shared_ptr_t<>.
// #TODO Add debug feature to verify counter is not touched from 2 different threads.
template<typename Integer> class shared_counter_i
{
public:
    using integer_type = Integer;

    shared_counter_i() = default;
    shared_counter_i(const shared_counter_i &) = delete;
    shared_counter_i &operator=(const shared_counter_i &) = delete;
    ~shared_counter_i() { detail::Destructor_Ref_Check(m_counter); }

    integer_type AddRef() const { return detail::AddRef<integer_type>(m_counter); }
    integer_type Release() const { return detail::Release<integer_type>(m_counter); }
    integer_type UseCount() const { return m_counter; }

private:
    mutable integer_type m_counter = integer_type{ 0 };
};

// Additional aliases for convenience.

using RefCounterInteger = AtomicType32;

template<typename Derived, typename Deleter = NewDeleter<Derived>>
using atomic_intrusive_counter = atomic_intrusive_counter_i<RefCounterInteger, Derived, Deleter>;
template<typename Derived, typename Deleter = NewDeleter<Derived>>
using intrusive_counter = intrusive_counter_i<RefCounterInteger, Derived, Deleter>;

using atomic_shared_counter = atomic_shared_counter_i<RefCounterInteger>;
using shared_counter = shared_counter_i<RefCounterInteger>;

} // namespace rts
