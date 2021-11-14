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

namespace Thyme
{
namespace rts
{

// Return the file extension of a given string.
template<typename StringType> typename StringType::value_type *get_file_extension(StringType &filename)
{
    const char *begin = filename.begin();
    const char *end = filename.end() - 1;
    while (end != begin) {
        if (*end == '.')
            return end + 1;
        --end;
    }
    return end;
}

// Read from a file until the specified end character is reached. Will not stop reading at escaped end character. Expects
// string with reserved space for null terminator past the end. Always writes null terminator. Returns true if the end of the
// file was not yet reached.
template<typename StringType>
bool read_line(File *file,
    StringType &string,
    typename StringType::value_type eol_char = get_char<typename StringType::value_type>('\n'),
    typename StringType::size_type *num_copied = nullptr)
{
    using char_type = typename StringType::value_type;

    const char_type *begin = string.data();
    const char_type *end = begin + string.size();
    char_type *writer = string.data();

    int total_bytes_read = 0;
    bool escaped = false;

    while (writer != end) {
        const int bytes_read = file->Read(writer, sizeof(char_type));
        total_bytes_read += bytes_read;

        if (bytes_read != sizeof(char_type)) {
            break;
        }

        // Stop at end character.
        if (!escaped && *writer == eol_char) {
            break;
        }

        // End escaping.
        escaped = false;

        // Begin escaping.
        if (*writer == get_char<char_type>('\\')) {
            escaped = !escaped;
        }

        ++writer;
    }

    // Write null terminator.
    *writer = get_char<char_type>('\0');

    if (num_copied != nullptr) {
        *num_copied = (writer - begin) / sizeof(char_type);
    }

    return total_bytes_read != 0;
}

// Read any type from file.
template<typename T> bool read_any(File *file, T &value)
{
    return file->Read(&value, sizeof(T)) == sizeof(T);
}

// Write any type to file.
template<typename T> bool write_any(File *file, const T &value)
{
    return file->Write(&value, sizeof(T)) == sizeof(T);
}

// Read string buffer with given size from file.
template<typename StringType> bool read_str(File *file, StringType &string)
{
    using char_type = typename StringType::value_type;
    using size_type = typename StringType::size_type;

    size_type size = string.size();
    void *data = string.data();
    const int bytes = static_cast<int>(size) * sizeof(char_type);

    return file->Read(data, bytes) == bytes;
}

// Write string buffer with given size to file.
template<typename StringType> bool write_str(File *file, const StringType &string)
{
    using char_type = typename StringType::value_type;
    using size_type = typename StringType::size_type;

    size_type size = string.size();
    const void *data = string.data();
    const int bytes = static_cast<int>(size) * sizeof(char_type);

    return file->Write(data, bytes) == bytes;
}

// Read Bytes from file.
inline bool read(File *file, void *data, int len)
{
    return file->Read(data, len) == len;
}

// Write Bytes to file.
inline bool write(File *file, const void *data, int len)
{
    return file->Write(data, len) == len;
}

} // namespace rts
} // namespace Thyme
