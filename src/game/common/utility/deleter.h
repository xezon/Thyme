/**
 * @file
 *
 * @author xezon
 *
 * @brief General purpose template deleter. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

// #FEATURE Deleter for objects created by 'new'
template<typename Type> struct NewDeleter
{
    using DeleteType = Type;
    void operator()(Type *instance) const { delete instance; }
};

// #FEATURE Deleter for objects created by 'new[]'
template<typename Type> struct NewArrayDeleter
{
    using DeleteType = Type;
    void operator()(Type *instance) const { delete[] instance; }
};

// #FEATURE Deleter for objects created by 'malloc()'
template<typename Type> struct AllocDeleter
{
    using DeleteType = Type;
    void operator()(Type *instance) const { free(instance); }
};

// #FEATURE Deleter for objects created by 'NEW_POOL_OBJ'
template<typename Type> struct MemoryPoolObjectDeleter
{
    using DeleteType = Type;
    void operator()(Type *instance) const { instance->Delete_Instance(); }
};

template<typename Deleter> inline void Invoke_Deleter(typename Deleter::DeleteType *instance)
{
    Deleter deleter;
    deleter(instance);
}
