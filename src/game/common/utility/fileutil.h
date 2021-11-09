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
template<typename StringView> typename StringView::value_type *get_file_extension(StringView &filename)
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
template<typename StringView>
bool read_line(File *file,
    StringView &string,
    typename StringView::value_type eol_char = rts::get_char<typename StringView::value_type>('\n'),
    typename StringView::size_type *num_copied = nullptr)
{
    using char_type = typename StringView::value_type;

    char_type *begin = string.data();
    char_type *end = begin + string.size();
    char_type *it = begin;

    int total_bytes_read = 0;
    bool escaped = false;

    while (it != end) {
        const int bytes_read = file->Read(it, sizeof(char_type));
        total_bytes_read += bytes_read;

        if (bytes_read != sizeof(char_type)) {
            break;
        }

        // Stop at end character.
        if (!escaped && *it == eol_char) {
            break;
        }

        // End escaping.
        escaped = false;

        // Begin escaping.
        if (*it == get_char<char_type>('\\')) {
            escaped = !escaped;
        }

        ++it;
    }

    // Write null terminator.
    *it = get_char<char_type>('\0');

    if (num_copied != nullptr) {
        *num_copied = (it - begin) / sizeof(char_type);
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
template<typename StringView> bool read_str(File *file, StringView &string)
{
    using char_type = typename StringView::value_type;
    using size_type = typename StringView::size_type;

    size_type size = string.size();
    void *data = string.data();
    const int bytes = static_cast<int>(size) * sizeof(char_type);

    return file->Read(data, bytes) == bytes;
}

// Write string buffer with given size to file.
template<typename StringView> bool write_str(File *file, const StringView &string)
{
    using char_type = typename StringView::value_type;
    using size_type = typename StringView::size_type;

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
