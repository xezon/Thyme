/**
 * @file
 *
 * @author xezon
 *
 * @brief File utility functions and similar
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "common/system/file.h"
#include "stringutil.h"

namespace rts
{

template<typename T> bool Read_Any(File *file, T &value);
template<typename T> bool Write_Any(File *file, const T &value);

template<typename StringType> bool Read_Str(File *file, StringType &string);
template<typename StringType> bool Write_Str(File *file, const StringType &string);

inline bool Read(File *file, void *data, int len);
inline bool Write(File *file, const void *data, int len);

// Read any type from file.
template<typename T> bool Read_Any(File *file, T &value)
{
    return file->Read(&value, sizeof(T)) == sizeof(T);
}

// Write any type to file.
template<typename T> bool Write_Any(File *file, const T &value)
{
    return file->Write(&value, sizeof(T)) == sizeof(T);
}

// Read Utf8String or Utf16String from file.
template<typename StringType> bool Read_Str(File *file, StringType &string)
{
    using char_type = typename StringType::value_type;
    using size_type = typename StringType::size_type;

    size_type size = string.size();

    captainslog_assert(size >= 0);
    if (size == 0)
        return true;

    void *data = string.data();
    const int bytes = static_cast<int>(size) * sizeof(char_type);

    if (file->Read(data, bytes) == bytes) {
        return true;
    }
    return false;
}

// Write Utf8String or Utf16String to file.
template<typename StringType> bool Write_Str(File *file, const StringType &string)
{
    using char_type = typename StringType::value_type;
    using size_type = typename StringType::size_type;

    size_type size = string.size();
    const void *data = string.data();
    const int bytes = static_cast<int>(size) * sizeof(char_type);

    return file->Write(data, bytes) == bytes;
}

// Read Bytes from file.
inline bool Read(File *file, void *data, int len)
{
    return file->Read(data, len) == len;
}

// Write Bytes to file.
inline bool Write(File *file, const void *data, int len)
{
    return file->Write(data, len) == len;
}

} // namespace rts
