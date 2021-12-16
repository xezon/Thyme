/**
 * @file
 *
 * @author xezon
 *
 * @brief File Reference class to use with File class (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "utility/nonintrusiveptr.h"

class File;

namespace Thyme
{

// #FEATURE File deleter
struct FileDeleter
{
    using DeleteType = File;
    void operator()(File *instance) const;
};

// #FEATURE Reference counted file class that automatically closes the assigned file when all references are destroyed.
template<typename Counter> class FileRefT
{
public:
    FileRefT() : m_file(nullptr) {}

    ~FileRefT() {}

    FileRefT(File *file)
    {
        m_file = file;
        if (m_file)
            m_file->Set_Del_On_Close(false);
    }

    FileRefT(const FileRefT &other) { m_file = other.m_file; }

    FileRefT &operator=(File *file)
    {
        m_file = file;
        if (m_file)
            m_file->Set_Del_On_Close(false);
        return *this;
    }

    FileRefT &operator=(const FileRefT &other)
    {
        m_file = other.m_file;
        return *this;
    }

    File &operator*() { return *m_file; }
    File *operator->() { return m_file.get(); }
    File *Get() { return m_file.get(); }

    const File &operator*() const { return *m_file; }
    const File *operator->() const { return m_file.get(); }
    const File *Get() const { return m_file.get(); }

    // Returns whether or not file is open. Unopened file should not be referenced.
    bool Is_Open() const { return m_file.get() && m_file->m_access; }

private:
    rts::nonintrusive_ptr_t<File, FileDeleter, Counter> m_file;
};

using FileRef = FileRefT<rts::nonintrusive_counter>;
using FileRefAtomic = FileRefT<rts::nonintrusive_atomic_counter>;

} // namespace Thyme
