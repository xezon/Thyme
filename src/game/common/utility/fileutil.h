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

// Return the file extension of a given string.
template<typename StringType> const typename StringType::value_type *get_file_extension(const StringType &filename)
{
    using char_type = typename StringType::value_type;

    const char *begin = filename.begin();
    const char *end = filename.end() - 1;

    char_type ext_char = get_char<char_type>('.');
    char_type dir1_char = get_char<char_type>(':');
    char_type dir2_char = get_char<char_type>('/');
    char_type dir3_char = get_char<char_type>('\\');

    while (end != begin) {
        char_type curr_char = *end;
        if (curr_char == ext_char) {
            return end + 1;
        }
        if (curr_char == dir1_char || curr_char == dir2_char || curr_char == dir3_char) {
            return get_null_str<char_type>();
        }
        --end;
    }
    return get_null_str<char_type>();
}

// Read from a file until the specified end character is reached. Will not stop reading at escaped end character. Writes to
// destination string until size minus 1. Always writes null terminator. Returns true, unless no line was read and the end of
// the file was reached.
template<typename CharType>
bool read_line(File *file,
    CharType *dest,
    std::size_t size,
    std::size_t *num_copied = nullptr,
    CharType eol_char = get_char<CharType>('\n'))
{
    using char_type = CharType;

    const char_type *writer_end = dest + size - 1;
    char_type *writer = dest;

    int total_bytes_read = 0;
    bool escaped = false;

    while (writer != writer_end) {
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

    *writer = get_char<char_type>('\0');

    if (num_copied != nullptr) {
        *num_copied = (writer - dest);
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
