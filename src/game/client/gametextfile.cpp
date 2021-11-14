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
#include "common/utility/arrayutil.h"
#include "common/utility/fileutil.h"
#include "common/utility/stringutil.h"
#include "filesystem.h"
#include "gametextlookup.h"
#include "rtsutils.h"
#include <captainslog.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

namespace Thyme
{

namespace
{
template<typename CharType> rts::escaped_char_alias_view<CharType> Get_Escaped_Characters()
{
    static const rts::escaped_char_alias<CharType> escaped_chars[] = {
        { rts::get_char<CharType>('\n'), rts::get_char<CharType>('\\'), rts::get_char<CharType>('n') },
        { rts::get_char<CharType>('\t'), rts::get_char<CharType>('\\'), rts::get_char<CharType>('t') },
        { rts::get_char<CharType>('"'), rts::get_char<CharType>('\\'), rts::get_char<CharType>('"') },
        { rts::get_char<CharType>('?'), rts::get_char<CharType>('\\'), rts::get_char<CharType>('?') },
        { rts::get_char<CharType>('\''), rts::get_char<CharType>('\\'), rts::get_char<CharType>('\'') },
        { rts::get_char<CharType>('\\'), rts::get_char<CharType>('\\'), rts::get_char<CharType>('\\') },
    };
    return rts::escaped_char_alias_view<CharType>(escaped_chars);
}

const char s_eol[] = { '\r', '\n' };
const char s_quo[] = { '"' };
const char s_end[] = { 'E', 'N', 'D' };

} // namespace

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
    auto bufview_in = rts::stack_array_view(buffer_in);
    auto bufview_out = rts::stack_array_view(buffer_out);
    auto bufview_ex = rts::stack_array_view(buffer_ex);
    auto bufview_trans = rts::stack_array_view(buffer_trans);

    if (filetype == Type::CSF) {
        success = Read_CSF_File(file, m_stringInfos, m_language);
    } else if (filetype == Type::STR) {
        success = Read_STR_File(file, m_stringInfos);
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
        success = Write_CSF_File(file, m_stringInfos, m_language);
    } else if (filetype == Type::STR) {
        success = Write_STR_File(file, m_stringInfos);
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

GameTextFile::Type GameTextFile::Get_File_Type(const char *filename, Type filetype)
{
    if (filetype == Type::AUTO) {
        const char *fileext = rts::get_file_extension(rts::make_array_view(filename));
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

bool GameTextFile::Read_STR_File(FileRef &file, StringInfos &string_infos)
{
    captainslog_info("Reading string file '%s' in STR format", file->Get_File_Name().Str());

    char utf8buf[GAMETEXT_BUFFER_8_SIZE] = { 0 };
    auto utf8view = rts::stack_array_view(utf8buf);

    // Instead of reading the file once from top to bottom to get the number of the entries, we will allocate a very generous
    // buffer to begin with and shrink it down afterwards. This will reduce algorithm complexity and file access.
    StringInfo string_info;
    StringInfos tmp_string_infos;
    tmp_string_infos.reserve(16384);

    ReadStep step = READ_STEP_LABEL;
    size_t num_copied = 0;
    char eol = '\n';

    const auto escaped_chars_view = Get_Escaped_Characters<char>();

    while (rts::read_line(file.Get(), utf8view, eol, &num_copied)) {
        rts::replace_characters(utf8view.data(), "\t\n\v\f\r", ' ');
        num_copied = rts::strip_leading_and_trailing_spaces(utf8view.data());

        if (step == READ_STEP_LABEL) {
            if (num_copied == 0) {
                continue;
            }
            if (Is_STR_Comment(utf8view.data())) {
                continue;
            }
            string_info.label = utf8view.data();
        } else if (step == READ_STEP_TEXT_BEGIN) {
            // String until text begin can be empty.
        } else if (step == READ_STEP_TEXT_END) {
            // String until text end can be empty.
            // STR does support escaped characters for special control characters (\n, \t, ...)
            // When written out as 2 symbol sequence, it will be converted into single character here. Convert in place.
            rts::convert_from_escaped_characters(utf8view.data(), utf8view.data(), utf8view.size(), escaped_chars_view);
            // Strip any remaining obsolete spaces for cleaner presentation in game.
            rts::strip_obsolete_spaces(utf8view.data());

            string_info.text.Translate(utf8view.data());
        } else if (step == READ_STEP_END) {
            if (num_copied == 0) {
                continue;
            }
            if (Is_STR_Comment(utf8view.data())) {
                continue;
            }
            if (Is_STR_End(utf8view.data())) {
                // Is end. Push to container and continue with next label.
                tmp_string_infos.push_back(string_info);
            } else {
                // Is not end. Take speech string and continue looking for end.
                string_info.speech = utf8view.data();
                continue;
            }
        }

        Next_Step(step, eol);
    }

    // The final copy to keep memory foot print minimal.
    string_infos.insert(string_infos.end(), tmp_string_infos.begin(), tmp_string_infos.end());

    return !string_infos.empty();
}

bool GameTextFile::Is_STR_Comment(const char *cstring)
{
    return (cstring[0] == '\\' && cstring[1] == '\\');
}

bool GameTextFile::Is_STR_End(const char *cstring)
{
    return (strcasecmp(cstring, "END") == 0);
}

void GameTextFile::Next_Step(ReadStep &step, char &eol)
{
    if (++step > READ_STEP_END) {
        step = READ_STEP_LABEL;
    }

    switch (step) {
        case READ_STEP_LABEL:
            eol = '\n';
            break;
        case READ_STEP_TEXT_BEGIN:
            eol = '"';
            break;
        case READ_STEP_TEXT_END:
            eol = '"';
            break;
        case READ_STEP_END:
            eol = '\n';
            break;
    }
}

bool GameTextFile::Read_CSF_File(FileRef &file, StringInfos &string_infos, LanguageID &language)
{
    captainslog_info("Reading string file '%s' in CSF format", file->Get_File_Name().Str());

    bool success = false;

    if (Read_CSF_Header(file, string_infos, language)) {
        success = true;

        for (StringInfo &string_info : string_infos) {
            if (!Read_CSF_Entry(file, string_info)) {
                success = false;
                break;
            }
        }
    }
    return success;
}

bool GameTextFile::Read_CSF_Header(FileRef &file, StringInfos &string_infos, LanguageID &language)
{
    CSFHeader header;

    if (rts::read_any(file.Get(), header)) {
        letoh_ref(header.id);
        letoh_ref(header.langid);
        letoh_ref(header.num_labels);
        letoh_ref(header.num_strings);
        letoh_ref(header.skip);
        letoh_ref(header.version);

        if (header.id == rts::FourCC_LE<'C', 'S', 'F', ' '>::value) {
            language = (header.version > 1) ? static_cast<LanguageID>(header.langid) : LanguageID::LANGUAGE_ID_US;
            string_infos.resize(header.num_labels);
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

    if (rts::read_any(file.Get(), header)) {
        letoh_ref(header.id);
        letoh_ref(header.texts);
        letoh_ref(header.length);

        if (header.id == rts::FourCC_LE<'L', 'B', 'L', ' '>::value) {
            if (rts::read_str(file.Get(), rts::resized_array_view(string_info.label, header.length))) {
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

        if (rts::read_any(file.Get(), header)) {
            letoh_ref(header.id);
            letoh_ref(header.length);

            read_speech = (header.id == rts::FourCC_LE<'S', 'T', 'R', 'W'>::value);
            const bool read_text = (header.id == rts::FourCC_LE<'S', 'T', 'R', ' '>::value);

            if (read_speech || read_text) {
                auto str = rts::resized_array_view(string_info.text, header.length);

                if (rts::read_str(file.Get(), str)) {
                    for (int32_t i = 0; i < header.length; ++i) {
                        letoh_ref(string_info.text[i]);
                        // Every char is binary flipped here by design.
                        string_info.text[i] = ~string_info.text[i];
                    }

                    if (str.data() != nullptr) {
                        // Strip obsolete spaces for cleaner presentation in game.
                        rts::strip_obsolete_spaces(str.data());
                    }

                    text_ok = true;
                }
            }
        }
    }

    if (read_speech) {
        CSFSpeechHeader header;

        if (rts::read_any(file.Get(), header)) {
            letoh_ref(header.length);

            if (rts::read_str(file.Get(), rts::resized_array_view(string_info.speech, header.length))) {
                speech_ok = true;
            }
        }
    }

    return text_ok && (speech_ok || !read_speech);
}

bool GameTextFile::Write_STR_File(FileRef &file, const StringInfos &string_infos)
{
    captainslog_info("Writing string file '%s' in STR format", file->Get_File_Name().Str());

    unichar_t utf16buf[GAMETEXT_BUFFER_16_SIZE] = { 0 };
    auto utf16bufview = rts::stack_array_view(utf16buf);

    for (const StringInfo &string_info : string_infos) {
        if (!string_info.label.Is_Empty()) {
            if (!Write_STR_Entry(file, string_info, utf16bufview)) {
                return false;
            }
        }
    }
    return true;
}

bool GameTextFile::Write_STR_Entry(FileRef &file, const StringInfo &string_info, Utf16Buf utf16buf)
{
    if (Write_STR_Label(file, string_info)) {
        if (Write_STR_Text(file, string_info, utf16buf)) {
            if (Write_STR_Speech(file, string_info)) {
                if (Write_STR_End(file)) {
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
    ok = ok && rts::write_str(file.Get(), rts::make_array_view(string_info.label));
    ok = ok && rts::write_any(file.Get(), s_eol);
    return ok;
}

bool GameTextFile::Write_STR_Text(FileRef &file, const StringInfo &string_info, Utf16Buf utf16buf)
{
    // STR does support escaped characters for special control characters (\n, \t, ...)
    // Write them out as escaped characters so they are easily modifiable in text editor.
    const auto escaped_chars_view = Get_Escaped_Characters<unichar_t>();
    rts::convert_to_escaped_characters(utf16buf.data(), string_info.text.Str(), utf16buf.size(), escaped_chars_view);

    // Convert utf16 to utf8.
    Utf8String text_utf8;
    text_utf8.Translate(utf16buf.data()); // #TODO allocates each time

    bool ok = true;
    ok = ok && rts::write_any(file.Get(), s_quo);
    ok = ok && rts::write_str(file.Get(), rts::make_array_view(text_utf8));
    ok = ok && rts::write_any(file.Get(), s_quo);
    ok = ok && rts::write_any(file.Get(), s_eol);
    return ok;
}

bool GameTextFile::Write_STR_Speech(FileRef &file, const StringInfo &string_info)
{
    bool ok = true;
    if (!string_info.speech.Is_Empty()) {
        ok = ok && rts::write_str(file.Get(), rts::make_array_view(string_info.speech));
        ok = ok && rts::write_any(file.Get(), s_eol);
    }
    return ok;
}

bool GameTextFile::Write_STR_End(FileRef &file)
{
    bool ok = true;
    ok = ok && rts::write_any(file.Get(), s_end);
    ok = ok && rts::write_any(file.Get(), s_eol);
    ok = ok && rts::write_any(file.Get(), s_eol);
    return ok;
}

bool GameTextFile::Write_CSF_File(FileRef &file, const StringInfos &string_infos, const LanguageID &language)
{
    captainslog_info("Writing string file '%s' in CSF format", file->Get_File_Name().Str());

    bool success = false;

    if (Write_CSF_Header(file, string_infos, language)) {
        success = true;
        unichar_t utf16buf[GAMETEXT_BUFFER_16_SIZE] = { 0 };
        auto utf16bufview = rts::stack_array_view(utf16buf);
        int string_index = 0;

        for (const StringInfo &string_info : string_infos) {
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

bool GameTextFile::Write_CSF_Header(FileRef &file, const StringInfos &string_infos, const LanguageID &language)
{
    CSFHeader header;
    header.id = rts::FourCC_LE<'C', 'S', 'F', ' '>::value;
    header.version = 3;
    header.num_labels = string_infos.size();
    header.num_strings = string_infos.size();
    header.skip = rts::FourCC_LE<'T', 'H', 'Y', 'M'>::value;
    header.langid = language;
    htole_ref(header.id);
    htole_ref(header.version);
    htole_ref(header.num_labels);
    htole_ref(header.num_strings);
    htole_ref(header.skip);
    htole_ref(header.langid);

    return rts::write_any(file.Get(), header);
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
    header.id = rts::FourCC_LE<'L', 'B', 'L', ' '>::value;
    header.texts = 1;
    header.length = string_info.label.Get_Length();
    htole_ref(header.id);
    htole_ref(header.texts);
    htole_ref(header.length);

    if (rts::write_any(file.Get(), header)) {
        if (rts::write_str(file.Get(), rts::make_array_view(string_info.label))) {
            return true;
        }
    }
    return false;
}

bool GameTextFile::Write_CSF_Text(FileRef &file, const StringInfo &string_info, Utf16Buf utf16buf)
{
    bool text_ok = false;
    bool speech_ok = false;
    bool write_speech = !string_info.speech.Is_Empty();

    {
        size_t text_len = string_info.text.Get_Length();

        CSFTextHeader header;
        header.id = write_speech ? rts::FourCC_LE<'S', 'T', 'R', 'W'>::value : rts::FourCC_LE<'S', 'T', 'R', ' '>::value;
        header.length = text_len;
        htole_ref(header.id);
        htole_ref(header.length);

        if (rts::write_any(file.Get(), header)) {
            captainslog_dbgassert(utf16buf.size() >= text_len, "Buffer is expected to be larger or equal text");
            text_len = std::min(text_len, utf16buf.size());

            for (size_t i = 0; i < text_len; ++i) {
                // Every char is binary flipped here by design.
                utf16buf[i] = ~string_info.text[i];
                htole_ref(utf16buf[i]);
            }

            if (rts::write(file.Get(), utf16buf.data(), text_len * sizeof(Utf16Buf::value_type))) {
                text_ok = true;
            }
        }
    }

    if (text_ok && write_speech) {
        CSFSpeechHeader header;
        header.length = string_info.speech.Get_Length();
        htole_ref(header.length);

        if (rts::write_any(file.Get(), header)) {
            if (rts::write_str(file.Get(), rts::make_array_view(string_info.speech))) {
                speech_ok = true;
            }
        }
    }
    return text_ok && (speech_ok || !write_speech);
}

} // namespace Thyme
