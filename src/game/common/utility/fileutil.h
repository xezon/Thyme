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
template<typename T> bool Read_Any(FileRef &file, T &value);

template<typename T> bool Write_Any(File *file, const T &value);
template<typename T> bool Write_Any(FileRef &file, const T &value);

template<typename T> bool Read_Str(File *file, T &string, int len);
template<typename T> bool Read_Str(FileRef &file, T &string, int len);

template<typename T> bool Write_Str(File *file, const T &string);
template<typename T> bool Write_Str(FileRef &file, const T &string);

inline bool Read(File *file, void *data, int len);
inline bool Read(FileRef &file, void *data, int len);

inline bool Write(File *file, const void *data, int len);
inline bool Write(FileRef &file, const void *data, int len);

// Read any type from file.
template<typename T> bool Read_Any(File *file, T &value)
{
    return file->Read(&value, sizeof(T)) == sizeof(T);
}

// Read any type from file.
template<typename T> bool Read_Any(FileRef &file, T &value)
{
    return Read_Any(file.Get(), value);
}

// Write any type to file.
template<typename T> bool Write_Any(File *file, const T &value)
{
    return file->Write(&value, sizeof(T)) == sizeof(T);
}

// Write any type to file.
template<typename T> bool Write_Any(FileRef &file, const T &value)
{
    return Write_Any(file.Get(), value);
}

// Read Utf8String or Utf16String from file.
template<typename T> bool Read_Str(File *file, T &string, int len)
{
    using char_type = typename T::value_type;

    captainslog_assert(len >= 0);
    if (len == 0)
        return true;

    char_type *buf = string.Get_Buffer_For_Read(len);
    const int bytes = len * sizeof(char_type);
    if (file->Read(buf, len) == len) {
        buf[len] = rts::Get_Null<char_type>();
        return true;
    }
    return false;
}

// Read Utf8String or Utf16String from file.
template<typename T> bool Read_Str(FileRef &file, T &string, int len)
{
    return Read_Str(file.Get(), string, len);
}

// Write Utf8String or Utf16String to file.
template<typename T> bool Write_Str(File *file, const T &string)
{
    using char_type = typename T::value_type;

    const void *buf = string.Str();
    const int bytes = string.Get_Length() * sizeof(char_type);
    return file->Write(buf, bytes) == bytes;
}

// Write Utf8String or Utf16String to file.
template<typename T> bool Write_Str(FileRef &file, const T &string)
{
    return Write_Str(file.Get(), string);
}

// Read Bytes from file.
inline bool Read(File *file, void *data, int len)
{
    return file->Read(data, len) == len;
}

// Read Bytes from file.
inline bool Read(FileRef &file, void *data, int len)
{
    return Read(file.Get(), data, len);
}

// Write Bytes to file.
inline bool Write(File *file, const void *data, int len)
{
    return file->Write(data, len) == len;
}

// Write Bytes to file.
inline bool Write(FileRef &file, const void *data, int len)
{
    return Write(file.Get(), data, len);
}

} // namespace rts
