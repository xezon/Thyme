/**
 * @file
 *
 * @author xezon
 *
 * @brief Game Localization file (Thyme Feature).
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "always.h"
#include "gametextfile.h"
#include "filesystem.h"
#include "gametextlookup.h"
#include "rtsutils.h"
#include <captainslog.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

using rts::FourCC;
using rts::FourCC_LE;

const char GameTextFile::s_eol[] = { '\r', '\n' };
const char GameTextFile::s_quo[] = { '"' };
const char GameTextFile::s_end[] = { 'E', 'N', 'D' };

bool GameTextFile::Load(const char *filename, Type filetype)
{
    Unload();

    if (!filename || !filename[0]) {
        captainslog_error("String file without file name cannot be loaded");
        return false;
    }

    FileRef file = g_theFileSystem->Open(filename, File::READ | File::BINARY);
    if (!file.Is_Open()) {
        captainslog_error("String file '%s' cannot be opened for load", filename);
        return false;
    }

    bool success = false;

    filetype = Get_File_Type(filename, filetype);

    int string_count = 0;
    int max_label_len = 0;
    char buffer_in[512] = { 0 };
    char buffer_out[512] = { 0 };
    char buffer_ex[512] = { 0 };
    unichar_t buffer_trans[1024] = { 0 };
    auto bufview_in = BufferView<char>::Create(buffer_in);
    auto bufview_out = BufferView<char>::Create(buffer_out);
    auto bufview_ex = BufferView<char>::Create(buffer_ex);
    auto bufview_trans = BufferView<unichar_t>::Create(buffer_trans);

    if (filetype == Type::CSF) {
        success = Read_CSF_File(file);
    } else if (filetype == Type::STR) {
        if (Get_String_Count(filename, string_count, bufview_in, bufview_out, bufview_ex)) {
            m_stringInfos.resize(string_count);

            if (Parse_String_File(
                    filename, &m_stringInfos[0], max_label_len, bufview_trans, bufview_in, bufview_out, bufview_ex)) {
                success = true;
            }
        }
    }

    if (success) {
        const bool collect = static_cast<int>(m_options & GameTextOption::PRINT_LENGTH_INFO_ON_LOAD) != 0;
        if (collect) {
            LengthInfo len_info;
            Collect_Length_Info(len_info, m_stringInfos);
            Log_Length_Info(len_info);
            Check_Length_Info(len_info);
        }
        captainslog_info("String file '%s' with %zu lines loaded successfully", filename, m_stringInfos.size());
    } else {
        Unload();
        captainslog_info("String file '%s' failed to load", filename);
    }

    return success;
}

bool GameTextFile::Save(const char *filename, Type filetype)
{
    if (!filename || !filename[0]) {
        captainslog_error("String file without file name cannot be saved");
        return false;
    }

    if (m_stringInfos.empty()) {
        captainslog_error("String file without string data cannot be saved");
        return false;
    }

    FileRef file = g_theFileSystem->Open(filename, File::WRITE | File::CREATE | File::BINARY);
    if (!file.Is_Open()) {
        captainslog_error("String file '%s' cannot be opened for save", filename);
        return false;
    }

    bool success = false;

    filetype = Get_File_Type(filename, filetype);

    if (filetype == Type::CSF) {
        success = Write_CSF_File(file);
    } else if (filetype == Type::STR) {
        success = Write_STR_File(file);
    }

    if (success) {
        const bool collect = static_cast<int>(m_options & GameTextOption::PRINT_LENGTH_INFO_ON_SAVE) != 0;
        if (collect) {
            LengthInfo len_info;
            Collect_Length_Info(len_info, m_stringInfos);
            Log_Length_Info(len_info);
            Check_Length_Info(len_info);
        }
        captainslog_info("String file '%s' with %zu text lines saved successfully", filename, m_stringInfos.size());
    } else {
        captainslog_info("String file '%s' failed to save", filename);
    }

    return success;
}

void GameTextFile::Unload()
{
    m_language = LanguageID::LANGUAGE_ID_US;
    StringInfos().swap(m_stringInfos);
}

void GameTextFile::Merge_And_Overwrite(const GameTextFile &other)
{
    const size_t other_size = other.m_stringInfos.size();
    StringInfos other_merge;
    other_merge.reserve(other_size);

    MutableGameTextLookup this_lookup(m_stringInfos);

    for (const StringInfo &other_string : other.m_stringInfos) {
        const MutableStringLookup *this_string_lookup = this_lookup.Find(other_string.label.Str());

        if (this_string_lookup == nullptr) {
            // Other string is new. Prepare to add.
            other_merge.push_back(other_string);
        } else {
            // Other string already exists. Update this string.
            this_string_lookup->string_info->text = other_string.text;
            this_string_lookup->string_info->speech = other_string.speech;
        }
    }

    m_stringInfos.insert(m_stringInfos.end(), other_merge.begin(), other_merge.end());
}

void GameTextFile::Read_To_End_Of_Quote(File *file, char *in, char *out, char *wave, int buff_len)
{
    bool escape = false;
    int i;

    for (i = 0; i < buff_len; ++i) {
        char current;

        // if in pointer is valid, read data from that, otherwise read from file.
        if (in != nullptr) {
            current = *in++;

            if (current == '\0') {
                in = nullptr;
                current = Read_Char(file);
            }
        } else {
            current = Read_Char(file);
        }

        // -1 and we are done?
        if (current == '\xFF') {
            return;
        }

        // Handle some special characters.
        if (current == '\n') {
            escape = false;
            current = ' ';
        } else if (current == '\\') {
            escape = !escape;
        } else if (current == '"' && !escape) {
            break;
        } else {
            escape = false;
        }

        // Treat any white space as a space char.
        if (isspace(uint8_t(current))) {
            current = ' ';
        }

        // Copy to output buffer.
        out[i] = current;
    }

    out[i] = '\0';

    int wave_pos = 0;
    int state = 0;

    while (true) {
        char current;

        // If in pointer is valid, read data from that, otherwise read from file.
        if (in != nullptr) {
            current = *in++;

            if (current == '\0') {
                in = nullptr;
                current = Read_Char(file);
            }
        } else {
            current = Read_Char(file);
        }

        // Stop reading on line break or -1 char.
        if (current == '\n' || current == '\xFF') {
            break;
        }

        // state 0 ignores initial whitespace and '='
        if (state == 0 && !isspace(uint8_t(current)) && current != '=') {
            state = 1;
        }

        // state 1 read so long as its alphanumeric characters or underscore.
        if (state == 1) {
            if ((current < 'a' || current > 'z') && (current < 'A' || current > 'Z') && (current < '0' || current > '9')
                && current != '_') {
                state = 2;
            } else {
                wave[wave_pos++] = current;
            }
        }

        // state 2 ignore everything and keep reading until a breaking condition is encountered.
    }

    if (wave_pos > 0) {
        if (wave[wave_pos - 1] >= '0' && wave[wave_pos - 1] <= '9') {
            wave[wave_pos++] = 'e';
        }
    }

    wave[wave_pos] = '\0';
}

void GameTextFile::Translate_Copy(unichar_t *out, const char *in)
{
    // Unsigned allows handling Latin extended code page.
    const unsigned char *in_unsigned = reinterpret_cast<const unsigned char *>(in);
    unsigned char current = *in_unsigned++;
    bool escape = false;

    while (current != '\0') {
        // Last char was a '\', handle for escape sequence.
        if (escape) {
            escape = false;

            switch (current) {
                case '\\':
                    *out++ = U_CHAR('\\');
                    break;
                case '\'':
                    *out++ = U_CHAR('\'');
                    break;
                case '"':
                    *out++ = U_CHAR('"');
                    break;
                case '?':
                    *out++ = U_CHAR('?');
                    break;
                case 't':
                    *out++ = U_CHAR('\t');
                    break;
                case 'n':
                    *out++ = U_CHAR('\n');
                    break;
                case '\0':
                    return;
                default:
                    *out++ = current;
                    break;
            }
        } else {
            if (current == '\\') {
                // Set to handle escape sequence.
                escape = true;
            } else {
                // Otherwise its a naive copy, assumes input is pure ascii.
                *out++ = current;
            }
        }

        current = *in_unsigned++;
    }

    // Null terminate.
    *out = U_CHAR('\0');
}

void GameTextFile::Remove_Leading_And_Trailing(char *buffer)
{
    int first = 0;

    // Find first none whitespace char.
    while (buffer[first] != '\0' && isspace(uint8_t(buffer[first]))) {
        ++first;
    }

    int pos = 0;

    // Move data down to start of buffer.
    while (buffer[first] != '\0') {
        buffer[pos++] = buffer[first++];
    }

    // Move pos back to last none whitespace.
    while (--pos >= 0 && isspace(buffer[pos])) {
        // Empty loop.
    }

    // Null terminate after last none whitespace.
    buffer[pos + 1] = '\0';
}

void GameTextFile::Strip_Spaces(unichar_t *buffer)
{
    unichar_t *getp = buffer;
    unichar_t *putp = buffer;

    unichar_t current = *getp++;
    unichar_t last = U_CHAR('\0');

    bool prev_whitepsace = true;

    while (current != U_CHAR('\0')) {
        if (current == U_CHAR(' ')) {
            if (last == U_CHAR(' ') || prev_whitepsace) {
                current = *getp++;

                continue;
            }
        } else if (current == U_CHAR('\n') || current == U_CHAR('\t')) {
            if (last == U_CHAR(' ')) {
                --putp;
            }

            *putp++ = current;
            prev_whitepsace = true;
            last = current;
            current = *getp++;

            continue;
        }

        *putp++ = current;
        prev_whitepsace = false;
        last = current;
        current = *getp++;
    }

    if (last == U_CHAR(' ')) {
        --putp;
    }

    // Ensure we null terminate after the last shuffled character.
    *putp = U_CHAR('\0');
}

char GameTextFile::Read_Char(File *file)
{
    char tmp;

    if (file->Read(&tmp, sizeof(tmp)) == sizeof(tmp)) {
        return tmp;
    }

    return '\0';
}

bool GameTextFile::Read_Line(char *buffer, int length, File *file)
{
    bool ret = false;
    char *putp = buffer;

    for (int i = 0; i < length; ++i) {
        if (file->Read(putp, sizeof(*putp)) != sizeof(*putp)) {
            break;
        }

        ret = true;

        if (*putp == '\n') {
            break;
        }

        ++putp;
    }

    *putp = '\0';

    return ret;
}

bool GameTextFile::Get_String_Count(
    const char *filename, int &count, BufferView<char> buffer_in, BufferView<char> buffer_out, BufferView<char> buffer_ex)
{
    File *file = g_theFileSystem->Open(filename, File::TEXT | File::READ);
    count = 0;

    if (file == nullptr) {
        return false;
    }

    while (Read_Line(buffer_in, buffer_in.Size() - 1, file)) {
        Remove_Leading_And_Trailing(buffer_in);

        if (buffer_in[0] == '"') {
            size_t len = strlen(buffer_in);
            buffer_in[len] = '\n';
            buffer_in[len + 1] = '\0';
            Read_To_End_Of_Quote(file, &buffer_in[1], buffer_out, buffer_ex, buffer_out.Size());
        } else if (strcasecmp(buffer_in, "END") == 0) {
            ++count;
        }
    }

    count += 500; // #TODO Investigate

    file->Close();

    return true;
}

bool GameTextFile::Parse_String_File(const char *filename,
    StringInfo *string_info,
    int &max_label_len,
    BufferView<unichar_t> buffer_trans,
    BufferView<char> buffer_in,
    BufferView<char> buffer_out,
    BufferView<char> buffer_ex)
{
    captainslog_info("Parsing string file '%s'.", filename);
    File *file = g_theFileSystem->Open(filename, File::TEXT | File::READ);

    if (file == nullptr) {
        return false;
    }

    int index = 0;
    bool end = false;

    while (Read_Line(buffer_in, buffer_in.Size(), file)) {
        Remove_Leading_And_Trailing(buffer_in);
        captainslog_trace("We have '%s' buffered.", m_bufferIn);

        // Skip line if it is empty or is a comment starting with "//".
        if (buffer_in[0] == '\0' || (buffer_in[0] == '/' && buffer_in[1] == '/')) {
            captainslog_trace("Line started with // or empty line. Skip.");
            continue;
        }

        end = false;

#if ASSERT_LEVEL >= ASSERTS_DEBUG
        if (index > 0) {
            for (int i = 0; i < index; ++i) {
                captainslog_dbgassert(strcasecmp(string_info[i].label.Str(), buffer_in) != 0,
                    "String label '%s' is defined multiple times!",
                    buffer_in);
            }
        }
#endif

        string_info[index].label = buffer_in;
        max_label_len = std::max<int>(strlen(buffer_in), max_label_len);

        bool read_string = false;

        while (Read_Line(buffer_in, buffer_in.Size() - 1, file)) {
            Remove_Leading_And_Trailing(buffer_in);
            captainslog_trace("We have '%s' buffered.", m_bufferIn);

            if (buffer_in[0] == '"') {
                size_t len = strlen(buffer_in);
                buffer_in[len] = '\n';
                buffer_in[len + 1] = '\0';
                Read_To_End_Of_Quote(file, buffer_in + 1, buffer_out, buffer_ex, buffer_out.Size());

                if (read_string) {
                    captainslog_trace("String label '%s' has more than one string defined!", buffer_in);

                    continue;
                }

                Translate_Copy(buffer_trans, buffer_out);
                Strip_Spaces(buffer_trans);

                // TODO maybe Windows build does something extra here not done in mac version.

                string_info[index].text = buffer_trans;
                string_info[index].speech = buffer_ex;

                read_string = true;
            } else if (strcasecmp(buffer_in, "END") == 0) {
                ++index;
                end = true;

                break;
            }
        }
    }

    file->Close();

    if (!end) {
        captainslog_error("Unexpected end of string file '%s'.", filename);

        return false;
    }

    return true;
}

const char *GameTextFile::Get_File_Extension(const char *filename)
{
    const char *begin = filename;
    const char *end = filename + strlen(filename);
    while (end != begin) {
        if (*end == '.')
            return end + 1;
        --end;
    }
    return end;
}

GameTextFile::Type GameTextFile::Get_File_Type(const char *filename, Type filetype)
{
    if (filetype == Type::AUTO) {
        const char *fileext = Get_File_Extension(filename);
        if (strcasecmp(fileext, "csf") == 0)
            filetype = Type::CSF;
        else if (strcasecmp(fileext, "str") == 0)
            filetype = Type::STR;
        else
            filetype = Type::CSF;
    }
    return filetype;
}

void GameTextFile::Collect_Length_Info(LengthInfo &len_info, const StringInfos &strings)
{
    len_info.max_label_len = 0;
    len_info.max_text_utf8_len = 0;
    len_info.max_text_utf16_len = 0;
    len_info.max_speech_len = 0;

    Utf8String utf8text;

    for (const StringInfo &string : strings) {
        utf8text.Translate(string.text);

        len_info.max_label_len = std::max(len_info.max_label_len, string.label.Get_Length());
        len_info.max_text_utf8_len = std::max(len_info.max_text_utf8_len, string.text.Get_Length());
        len_info.max_text_utf16_len = std::max(len_info.max_text_utf16_len, string.text.Get_Length());
        len_info.max_speech_len = std::max(len_info.max_speech_len, string.speech.Get_Length());
    }
}

void GameTextFile::Log_Length_Info(const LengthInfo &len_info)
{
    captainslog_info("String file max label len: %d", len_info.max_label_len);
    captainslog_info("String file max utf8 text len: %d", len_info.max_text_utf8_len);
    captainslog_info("String file max utf16 text len: %d", len_info.max_text_utf16_len);
    captainslog_info("String file max speech len: %d", len_info.max_speech_len);
}

void GameTextFile::Check_Length_Info(const LengthInfo &len_info)
{
    captainslog_dbgassert(GAMETEXT_BUFFER_8_SIZE > len_info.max_label_len, "Read buffer must be larger");
    captainslog_dbgassert(GAMETEXT_BUFFER_8_SIZE > len_info.max_text_utf8_len, "Read buffer must be larger");
    captainslog_dbgassert(GAMETEXT_BUFFER_16_SIZE > len_info.max_text_utf16_len, "Read buffer must be larger");
    captainslog_dbgassert(GAMETEXT_BUFFER_8_SIZE > len_info.max_speech_len, "Read buffer must be larger");
}

template<typename T> static bool GameTextFile::Read(FileRef &file, T &value)
{
    return file->Read(&value, sizeof(T)) == sizeof(T);
}

bool GameTextFile::Read(FileRef &file, Utf8String &string, int len)
{
    captainslog_assert(len >= 0);
    if (len == 0)
        return true;

    char *buf = string.Get_Buffer_For_Read(len);
    const int bytes = len * sizeof(char);
    if (file->Read(buf, len) == len) {
        buf[len] = '\0';
        return true;
    }
    return false;
}

bool GameTextFile::Read(FileRef &file, Utf16String &string, int len)
{
    captainslog_assert(len >= 0);
    if (len == 0)
        return true;

    unichar_t *buf = string.Get_Buffer_For_Read(len);
    const int bytes = len * sizeof(unichar_t);
    if (file->Read(buf, bytes) == bytes) {
        buf[len] = U_CHAR('\0');
        return true;
    }
    return false;
}

bool GameTextFile::Read(FileRef &file, void *data, int len)
{
    return file->Read(data, len) == len;
}

template<typename T> static bool GameTextFile::Write(FileRef &file, const T &value)
{
    return file->Write(&value, sizeof(T)) == sizeof(T);
}

template<> bool GameTextFile::Write<Utf8String>(FileRef &file, const Utf8String &string)
{
    const void *buf = string.Str();
    const int bytes = string.Get_Length() * sizeof(char);
    return file->Write(buf, bytes) == bytes;
}

template<> bool GameTextFile::Write<Utf16String>(FileRef &file, const Utf16String &string)
{
    const void *buf = string.Str();
    const int bytes = string.Get_Length() * sizeof(unichar_t);
    return file->Write(buf, bytes) == bytes;
}

bool GameTextFile::Write(FileRef &file, const void *data, int len)
{
    return file->Write(data, len) == len;
}

bool GameTextFile::Read_CSF_File(FileRef &file)
{
    captainslog_info("Reading string file '%s' in CSF format", file->Get_File_Name().Str());

    bool success = false;

    if (Read_CSF_Header(file, m_language, m_stringInfos)) {
        success = true;

        for (StringInfo &string_info : m_stringInfos) {
            if (!Read_CSF_Entry(file, string_info)) {
                success = false;
                break;
            }
        }
    }
    return success;
}

bool GameTextFile::Read_CSF_Header(FileRef &file, LanguageID &language, StringInfos &strings)
{
    CSFHeader header;

    if (Read(file, header)) {
        letoh_ref(header.id);
        letoh_ref(header.langid);
        letoh_ref(header.num_labels);
        letoh_ref(header.num_strings);
        letoh_ref(header.skip);
        letoh_ref(header.version);

        if (header.id == FourCC_LE<'C', 'S', 'F', ' '>::value) {
            language = (header.version > 1) ? static_cast<LanguageID>(header.langid) : LanguageID::LANGUAGE_ID_US;
            strings.resize(header.num_labels);
            return true;
        }
    }

    return false;
}

bool GameTextFile::Read_CSF_Entry(FileRef &file, StringInfo &string_info)
{
    int32_t texts;

    if (Read_CSF_Label(file, string_info, texts)) {
        if (texts == 0) {
            return true;
        }
        if (Read_CSF_Text(file, string_info)) {
            return true;
        }
    }
    return false;
}

bool GameTextFile::Read_CSF_Label(FileRef &file, StringInfo &string_info, int32_t &texts)
{
    CSFLabelHeader header;

    if (Read(file, header)) {
        letoh_ref(header.id);
        letoh_ref(header.texts);
        letoh_ref(header.length);

        if (header.id == FourCC_LE<'L', 'B', 'L', ' '>::value) {
            if (Read(file, string_info.label, header.length)) {
                texts = header.texts;
                return true;
            }
        }
    }

    return false;
}

bool GameTextFile::Read_CSF_Text(FileRef &file, StringInfo &string_info)
{
    bool text_ok = false;
    bool speech_ok = false;
    bool read_speech = false;

    {
        CSFTextHeader header;

        if (Read(file, header)) {
            letoh_ref(header.id);
            letoh_ref(header.length);

            read_speech = (header.id == FourCC_LE<'S', 'T', 'R', 'W'>::value);
            const bool read_text = (header.id == FourCC_LE<'S', 'T', 'R', ' '>::value);

            if (read_speech || read_text) {
                if (Read(file, string_info.text, header.length)) {
                    for (int32_t i = 0; i < header.length; ++i) {
                        letoh_ref(string_info.text[i]);
                        // Every char is binary flipped here by design.
                        string_info.text[i] = ~string_info.text[i];
                    }
                    text_ok = true;
                }
            }
        }
    }

    if (read_speech) {
        CSFSpeechHeader header;

        if (Read(file, header)) {
            letoh_ref(header.length);

            if (Read(file, string_info.speech, header.length)) {
                speech_ok = true;
            }
        }
    }

    return text_ok && (speech_ok || !read_speech);
}

bool GameTextFile::Write_STR_File(FileRef &file)
{
    captainslog_info("Writing string file '%s' in STR format", file->Get_File_Name().Str());

    for (const StringInfo &string_info : m_stringInfos) {
        if (!string_info.label.Is_Empty()) {
            if (!Write_STR_Entry(file, string_info, m_options)) {
                return false;
            }
        }
    }
    return true;
}

bool GameTextFile::Write_STR_Entry(FileRef &file, const StringInfo &string_info, Option options)
{
    if (Write_STR_Label(file, string_info)) {
        if (Write_STR_Text(file, string_info, options)) {
            if (Write_STR_Speech(file, string_info)) {
                if (Write_STR_End(file, string_info)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool GameTextFile::Write_STR_Label(FileRef &file, const StringInfo &string_info)
{
    bool ok = true;
    ok = ok && Write(file, string_info.label);
    ok = ok && Write(file, s_eol);
    return ok;
}

bool GameTextFile::Write_STR_Text(FileRef &file, const StringInfo &string_info, Option options)
{
    Utf16String text_utf16;
    Utf8String text_utf8;

    // Copy utf16 text and treat certain characters special.
    const bool write_lf = static_cast<int>(options & Option::WRITEOUT_LF) != 0;

    for (int i = 0, count = string_info.text.Get_Length(); i < count; ++i) {
        unichar_t c = string_info.text[i];
        if (write_lf && c == U_CHAR('\n')) {
            // Print an escaped line feed.
            text_utf16.Concat(U_CHAR('\\'));
            text_utf16.Concat(U_CHAR('n'));
            if (i != 0) {
                // This is optional. Makes it easier to look at string in file.
                text_utf16.Concat(U_CHAR('\n'));
            }
        } else if (c == U_CHAR('"')) {
            // Print an escaped quote.
            text_utf16.Concat(U_CHAR('\\'));
            text_utf16.Concat(U_CHAR('"'));
        } else {
            text_utf16.Concat(c);
        }
    }

    // Convert utf16 to utf8.
    text_utf8.Translate(text_utf16);

    bool ok = true;
    ok = ok && Write(file, s_quo);
    ok = ok && Write(file, text_utf8);
    ok = ok && Write(file, s_quo);
    ok = ok && Write(file, s_eol);
    return ok;
}

bool GameTextFile::Write_STR_Speech(FileRef &file, const StringInfo &string_info)
{
    bool ok = true;
    if (!string_info.speech.Is_Empty()) {
        ok = ok && Write(file, string_info.speech);
        ok = ok && Write(file, s_eol);
    }
    return ok;
}

bool GameTextFile::Write_STR_End(FileRef &file, const StringInfo &string_info)
{
    bool ok = true;
    ok = ok && Write(file, s_end);
    ok = ok && Write(file, s_eol);
    ok = ok && Write(file, s_eol);
    return ok;
}

bool GameTextFile::Write_CSF_File(FileRef &file)
{
    captainslog_info("Writing string file '%s' in CSF format", file->Get_File_Name().Str());

    bool success = false;

    if (Write_CSF_Header(file, m_language, m_stringInfos)) {
        success = true;
        unichar_t utf16buf[GAMETEXT_BUFFER_16_SIZE];
        auto utf16bufview = Utf16Buf::Create(utf16buf);
        int string_index = 0;

        for (const StringInfo &string_info : m_stringInfos) {
            ++string_index;

            if (string_info.label.Is_Empty()) {
                captainslog_error("String %d has no label", string_index);
                continue;
            }

            if (!Write_CSF_Entry(file, string_info, utf16bufview)) {
                success = false;
                break;
            }
        }
    }
    return success;
}

bool GameTextFile::Write_CSF_Header(FileRef &file, const LanguageID &language, const StringInfos &strings)
{
    CSFHeader header;
    header.id = FourCC_LE<'C', 'S', 'F', ' '>::value;
    header.version = 3;
    header.num_labels = strings.size();
    header.num_strings = strings.size();
    header.skip = FourCC_LE<'T', 'H', 'Y', 'M'>::value;
    header.langid = language;
    htole_ref(header.id);
    htole_ref(header.version);
    htole_ref(header.num_labels);
    htole_ref(header.num_strings);
    htole_ref(header.skip);
    htole_ref(header.langid);

    return Write(file, header);
}

bool GameTextFile::Write_CSF_Entry(FileRef &file, const StringInfo &string_info, Utf16Buf utf16bufview)
{
    if (Write_CSF_Label(file, string_info)) {
        if (Write_CSF_Text(file, string_info, utf16bufview)) {
            return true;
        }
    }
    return false;
}

bool GameTextFile::Write_CSF_Label(FileRef &file, const StringInfo &string_info)
{
    CSFLabelHeader header;
    header.id = FourCC_LE<'L', 'B', 'L', ' '>::value;
    header.texts = 1;
    header.length = string_info.label.Get_Length();
    htole_ref(header.id);
    htole_ref(header.texts);
    htole_ref(header.length);

    if (Write(file, header)) {
        if (Write(file, string_info.label)) {
            return true;
        }
    }
    return false;
}

bool GameTextFile::Write_CSF_Text(FileRef &file, const StringInfo &string_info, Utf16Buf utf16bufview)
{
    bool text_ok = false;
    bool speech_ok = false;
    bool write_speech = !string_info.speech.Is_Empty();

    {
        int text_len = string_info.text.Get_Length();

        CSFTextHeader header;
        header.id = write_speech ? FourCC_LE<'S', 'T', 'R', 'W'>::value : FourCC_LE<'S', 'T', 'R', ' '>::value;
        header.length = text_len;
        htole_ref(header.id);
        htole_ref(header.length);

        if (Write(file, header)) {
            captainslog_dbgassert(utf16bufview.Size() >= text_len, "Buffer is expected to be larger or equal text");
            text_len = std::min<int>(text_len, utf16bufview.Size());

            for (int i = 0; i < text_len; ++i) {
                // Every char is binary flipped here by design.
                utf16bufview[i] = ~string_info.text[i];
                htole_ref(utf16bufview[i]);
            }

            if (Write(file, utf16bufview.Get(), text_len * sizeof(unichar_t))) {
                text_ok = true;
            }
        }
    }

    if (text_ok && write_speech) {
        CSFSpeechHeader header;
        header.length = string_info.speech.Get_Length();
        htole_ref(header.length);

        if (Write(file, header)) {
            if (Write(file, string_info.speech)) {
                speech_ok = true;
            }
        }
    }
    return text_ok && (speech_ok || !write_speech);
}
