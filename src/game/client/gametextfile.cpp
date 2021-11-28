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
#include "common/utility/stlutil.h"
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
template<typename CharType> rts::escaped_char_alias_view<CharType> Escaped_Characters_For_STR_Read()
{
    static CHAR_TRAITS_CONSTEXPR rts::escaped_char_alias<CharType> escaped_chars[] = {
        rts::escaped_char_alias<CharType>::make_real_alias2('\n', '\\', 'n'),
        rts::escaped_char_alias<CharType>::make_real_alias2('\t', '\\', 't'),
        rts::escaped_char_alias<CharType>::make_real_alias2('"', '\\', '"'),
        rts::escaped_char_alias<CharType>::make_real_alias2('?', '\\', '?'),
        rts::escaped_char_alias<CharType>::make_real_alias2('\'', '\\', '\''),
        rts::escaped_char_alias<CharType>::make_real_alias2('\\', '\\', '\\'),
    };
    return rts::escaped_char_alias_view<CharType>(escaped_chars);
}

template<typename CharType> rts::escaped_char_alias_view<CharType> Escaped_Characters_For_STR_Write()
{
    static CHAR_TRAITS_CONSTEXPR rts::escaped_char_alias<CharType> escaped_chars[] = {
        rts::escaped_char_alias<CharType>::make_real_alias2('\n', '\\', 'n'),
        rts::escaped_char_alias<CharType>::make_real_alias2('\t', '\\', 't'),
        rts::escaped_char_alias<CharType>::make_real_alias2('"', '\\', '"'),
        rts::escaped_char_alias<CharType>::make_real_alias2('\\', '\\', '\\'),
    };
    return rts::escaped_char_alias_view<CharType>(escaped_chars);
}

// ISO 639-1 language codes - sort of.

constexpr const char *const s_langcode_us = "US";
constexpr const char *const s_langcode_en = "EN";
constexpr const char *const s_langcode_de = "DE";
constexpr const char *const s_langcode_fr = "FR";
constexpr const char *const s_langcode_es = "ES";
constexpr const char *const s_langcode_it = "IT";
constexpr const char *const s_langcode_ja = "JA";
constexpr const char *const s_langcode_jb = "JB";
constexpr const char *const s_langcode_ko = "KO";
constexpr const char *const s_langcode_zh = "ZH";
constexpr const char *const s_langcode___ = "";
constexpr const char *const s_langcode_bp = "BP";
constexpr const char *const s_langcode_pl = "PL";
constexpr const char *const s_langcode_ru = "RU";
constexpr const char *const s_langcode_ar = "AR";

constexpr const char *const s_langcodes[] = {
    s_langcode_us,
    s_langcode_en,
    s_langcode_de,
    s_langcode_fr,
    s_langcode_es,
    s_langcode_it,
    s_langcode_ja,
    s_langcode_jb,
    s_langcode_ko,
    s_langcode_zh,
    s_langcode___,
    s_langcode_bp,
    s_langcode_pl,
    s_langcode___,
    s_langcode_ru,
    s_langcode_ar,
};

static_assert(s_langcode_us == s_langcodes[size_t(LanguageID::US)], "Expected language is not set");
static_assert(s_langcode_en == s_langcodes[size_t(LanguageID::UK)], "Expected language is not set");
static_assert(s_langcode_de == s_langcodes[size_t(LanguageID::GERMAN)], "Expected language is not set");
static_assert(s_langcode_fr == s_langcodes[size_t(LanguageID::FRENCH)], "Expected language is not set");
static_assert(s_langcode_es == s_langcodes[size_t(LanguageID::SPANISH)], "Expected language is not set");
static_assert(s_langcode_it == s_langcodes[size_t(LanguageID::ITALIAN)], "Expected language is not set");
static_assert(s_langcode_ja == s_langcodes[size_t(LanguageID::JAPANESE)], "Expected language is not set");
static_assert(s_langcode_jb == s_langcodes[size_t(LanguageID::JABBER)], "Expected language is not set");
static_assert(s_langcode_ko == s_langcodes[size_t(LanguageID::KOREAN)], "Expected language is not set");
static_assert(s_langcode_zh == s_langcodes[size_t(LanguageID::CHINESE)], "Expected language is not set");
static_assert(s_langcode___ == s_langcodes[size_t(LanguageID::UNUSED_1)], "Expected language is not set");
static_assert(s_langcode_bp == s_langcodes[size_t(LanguageID::BRAZILIAN)], "Expected language is not set");
static_assert(s_langcode_pl == s_langcodes[size_t(LanguageID::POLISH)], "Expected language is not set");
static_assert(s_langcode___ == s_langcodes[size_t(LanguageID::UNKNOWN)], "Expected language is not set");
static_assert(s_langcode_ru == s_langcodes[size_t(LanguageID::RUSSIAN)], "Expected language is not set");
static_assert(s_langcode_ar == s_langcodes[size_t(LanguageID::ARABIC)], "Expected language is not set");

static_assert(ARRAY_SIZE(s_langcodes) == size_t(LanguageID::COUNT), "Expected language is not set");

constexpr const char *Get_Language_Code(LanguageID language)
{
    return s_langcodes[size_t(language)];
}

constexpr const char s_str_eol[] = { '\r', '\n' };
constexpr const char s_str_quo[] = { '"' };
constexpr const char s_str_end[] = { 'E', 'N', 'D' };
constexpr const char s_str_lng[] = { ':', ' ' };

// https://www.rfc-editor.org/rfc/rfc3629
// In UTF-8, characters from the U+0000..U+10FFFF range (the UTF-16
// accessible range) are encoded using sequences of 1 to 4 octets.
constexpr size_t GAMETEXT_BUFFER_16_SIZE = 1024;
constexpr size_t GAMETEXT_BUFFER_8_SIZE = GAMETEXT_BUFFER_16_SIZE * 4;

struct CSFLabelHeader
{
    uint32_t id;
    int32_t texts;
    int32_t length;
};

struct CSFTextHeader
{
    uint32_t id;
    int32_t length;
};

struct CSFSpeechHeader
{
    int32_t length;
};

} // namespace

bool GameTextFile::Is_Loaded() const
{
    return !Get_String_Infos().empty();
}

bool GameTextFile::Is_Loaded(Languages languages) const
{
    bool loaded = true;
    For_Each_Language(languages, [&](LanguageID language) { loaded &= !Get_String_Infos(language).empty(); });
    return loaded;
}

bool GameTextFile::Load(const char *filename)
{
    Type filetype = Get_File_Type(filename, Type::AUTO);
    return Load(filename, filetype, nullptr, nullptr);
}

bool GameTextFile::Load_CSF(const char *filename)
{
    return Load(filename, Type::CSF, nullptr, nullptr);
}

bool GameTextFile::Load_STR(const char *filename, const Languages *languages, const size_t *size_hint)
{
    return Load(filename, Type::STR, languages, size_hint);
}

bool GameTextFile::Save(const char *filename)
{
    Type filetype = Get_File_Type(filename, Type::AUTO);
    return Save(filename, filetype, nullptr);
}

bool GameTextFile::Save_CSF(const char *filename)
{
    return Save(filename, Type::CSF, nullptr);
}

bool GameTextFile::Save_STR(const char *filename, const Languages *languages)
{
    return Save(filename, Type::STR, languages);
}

bool GameTextFile::Load(const char *filename, Type filetype, const Languages *languages, const size_t *size_hint)
{
    captainslog_assert(filetype != Type::AUTO);

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

    StringInfos string_infos;
    LanguageID language = m_language;

    if (filetype == Type::CSF) {
        success = Read_CSF_File(file, string_infos, language);
    } else if (filetype == Type::STR) {
        success = Read_STR_File(file, string_infos, m_options, size_hint);
    }

    if (success) {
        m_language = language;
        Mutable_String_Infos().swap(string_infos);

        if (m_options.has(Options::Value::CHECK_BUFFER_LENGTH_ON_LOAD)) {
            LengthInfo len_info;
            Collect_Length_Info(len_info, Get_String_Infos());
            Log_Length_Info(len_info);
            Check_Length_Info(len_info);
        }
        captainslog_info("String file '%s' with %zu lines loaded successfully", filename, Get_String_Infos().size());
    } else {
        captainslog_info("String file '%s' failed to load", filename);
    }

    return success;
}

bool GameTextFile::Save(const char *filename, Type filetype, const Languages *languages)
{
    captainslog_assert(filetype != Type::AUTO);

    if (!filename || !filename[0]) {
        captainslog_error("String file without file name cannot be saved");
        return false;
    }

    const Languages used_languages = (languages == nullptr) ? m_language : *languages;

    if (!Is_Loaded(used_languages)) {
        captainslog_error("String file without string data cannot be saved");
        return false;
    }

    FileRef file = g_theFileSystem->Open(filename, File::WRITE | File::CREATE | File::BINARY);
    if (!file.Is_Open()) {
        captainslog_error("String file '%s' cannot be opened for save", filename);
        return false;
    }

    bool success = false;

    if (filetype == Type::CSF) {
        success = Write_CSF_File(file, Get_String_Infos(), m_language);
    } else if (filetype == Type::STR) {
        if (languages != nullptr) {
            success = Write_STR_Multi_File(file, ConstStringInfosView(m_stringInfos), *languages, m_options);
        } else {
            success = Write_STR_File(file, Get_String_Infos(), m_options);
        }
    }

    if (success) {
        if (m_options.has(Options::Value::CHECK_BUFFER_LENGTH_ON_SAVE)) {
            Check_Buffer_Lengths(used_languages);
        }
        captainslog_info("String file '%s' with %zu text lines saved successfully", filename, Get_String_Infos().size());
    } else {
        captainslog_info("String file '%s' failed to save", filename);
    }

    return success;
}

void GameTextFile::Unload()
{
    Unload(m_language);
}

void GameTextFile::Unload(Languages languages)
{
    For_Each_Language(languages, [&](LanguageID language) { rts::free_container(Mutable_String_Infos(language)); });
}

void GameTextFile::Reset()
{
    for (StringInfos &string_infos : m_stringInfos) {
        rts::free_container(string_infos);
    }
    m_options = Options::Value::NONE;
    m_language = LanguageID::UNKNOWN;
}

void GameTextFile::Merge_And_Overwrite(const GameTextFile &other)
{
    Merge_And_Overwrite_Internal(other, m_language);
}

void GameTextFile::Merge_And_Overwrite(const GameTextFile &other, Languages languages)
{
    For_Each_Language(languages, [&](LanguageID language) { Merge_And_Overwrite_Internal(other, language); });
}

void GameTextFile::Merge_And_Overwrite_Internal(const GameTextFile &other, LanguageID language)
{
    const size_t other_size = other.Get_String_Infos(language).size();
    StringInfos &this_strings = Mutable_String_Infos(language);
    StringInfos other_new_strings;
    other_new_strings.reserve(other_size);

    {
        const MutableGameTextLookup this_lookup(this_strings);

        for (const StringInfo &other_string : other.Get_String_Infos(language)) {
            const MutableStringLookup *this_string_lookup = this_lookup.Find(other_string.label.Str());

            if (this_string_lookup == nullptr) {
                // Other string is new. Prepare to add.
                other_new_strings.push_back(other_string);
            } else {
                // Other string already exists. Update this string.
                this_string_lookup->string_info->text = other_string.text;
                this_string_lookup->string_info->speech = other_string.speech;
            }
        }
    }

    rts::append_container(this_strings, other_new_strings);
}

void GameTextFile::Check_Buffer_Lengths(Languages languages)
{
    For_Each_Language(languages, [this](LanguageID language) {
        LengthInfo len_info;
        Collect_Length_Info(len_info, Get_String_Infos(language));
        Log_Length_Info(len_info);
        Check_Length_Info(len_info);
    });
}

const StringInfos &GameTextFile::Get_String_Infos() const
{
    return m_stringInfos[size_t(m_language)];
}

const StringInfos &GameTextFile::Get_String_Infos(LanguageID language) const
{
    return m_stringInfos[size_t(language)];
}

StringInfos &GameTextFile::Mutable_String_Infos()
{
    return m_stringInfos[size_t(m_language)];
}

StringInfos &GameTextFile::Mutable_String_Infos(LanguageID language)
{
    return m_stringInfos[size_t(language)];
}

void GameTextFile::Set_Options(Options options)
{
    m_options = options;
}

GameTextFile::Options GameTextFile::Get_Options() const
{
    return m_options;
}

void GameTextFile::Set_Language(LanguageID language)
{
    m_language = language;
}

LanguageID GameTextFile::Get_Language() const
{
    return m_language;
}

void GameTextFile::Swap_String_Infos(LanguageID left, LanguageID right)
{
    if (left != right) {
        m_stringInfos[size_t(left)].swap(m_stringInfos[size_t(right)]);
    }
}

template<typename Functor> static void GameTextFile::For_Each_Language(Languages languages, Functor functor)
{
    for (rts::enumerator<LanguageID> it; it < LanguageID::COUNT; ++it) {
        if (languages.has(it.value())) {
            functor(it.value());
        }
    }
}

size_t GameTextFile::Get_Max_Size(ConstStringInfosPtrView string_infos_ptrs)
{
    size_t size = 0;
    for (const StringInfos *string_infos : string_infos_ptrs) {
        if (string_infos != nullptr) {
            size = std::max(size, string_infos->size());
        }
    }
    return size;
}

void GameTextFile::Build_Multi_String_Infos(
    MultiStringInfos &multi_string_infos, ConstStringInfosPtrView string_infos_ptrs, Options options)
{
    const size_t estimated_size = Get_Max_Size(string_infos_ptrs);
    size_t current_size = 0;
    MultiStringInfos tmp_multi_string_infos;
    tmp_multi_string_infos.reserve(estimated_size);
    multi_string_infos.clear();
    multi_string_infos.reserve(estimated_size);
    MutableMultiGameTextLookup lookup;

    size_t language_index = 0;

    for (const StringInfos *string_infos : string_infos_ptrs) {
        if (string_infos != nullptr) {
            if (current_size != multi_string_infos.size()) {
                current_size = multi_string_infos.size();
                lookup.Load(multi_string_infos);
            }

            for (const StringInfo &string_info : *string_infos) {
                const MutableMultiStringLookup *multi_string_lookup = lookup.Find(string_info.label.Str());

                if (multi_string_lookup == nullptr) {
                    MultiStringInfo multi_string_info;
                    multi_string_info.label = string_info.label;
                    multi_string_info.text[language_index] = string_info.text;
                    multi_string_info.speech[language_index] = string_info.speech;
                    tmp_multi_string_infos.push_back(multi_string_info);
                } else {
                    multi_string_lookup->string_info->text[language_index] = string_info.text;
                    multi_string_lookup->string_info->speech[language_index] = string_info.speech;
                }
            }
            rts::append_container(multi_string_infos, tmp_multi_string_infos);
            tmp_multi_string_infos.clear();
        }
        ++language_index;
    }

    if (options.has(Options::Value::OPTIMIZE_MEMORY_SIZE)) {
        rts::shrink_to_fit(multi_string_infos);
    }
}

void GameTextFile::Build_String_Infos(
    StringInfosPtrView string_infos_ptrs, const MultiStringInfos &multi_string_infos, Options options)
{
    size_t language_index = 0;

    for (StringInfos *string_infos : string_infos_ptrs) {
        if (string_infos != nullptr) {
            if (options.has(Options::Value::OPTIMIZE_MEMORY_SIZE)) {
                rts::free_container(*string_infos);
            } else {
                string_infos->clear();
            }
            string_infos->reserve(multi_string_infos.size());

            for (const MultiStringInfo &multi_string_info : multi_string_infos) {
                StringInfo string_info;
                string_info.label = multi_string_info.label;
                string_info.text = multi_string_info.text[language_index];
                string_info.speech = multi_string_info.speech[language_index];
                string_infos->push_back(string_info);
            }
        }
        ++language_index;
    }
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

bool GameTextFile::Read_STR_File(FileRef &file, StringInfos &string_infos, Options options, const size_t *size_hint)
{
    captainslog_info("Reading string file '%s' in STR format", file->Get_File_Name().Str());

    char read_buf[GAMETEXT_BUFFER_8_SIZE] = { 0 };
    Utf8View read8 = read_buf;

    // Instead of reading the file once from top to bottom to get the number of the entries, we will allocate a very generous
    // buffer to begin with and shrink it down afterwards. This will reduce algorithm complexity and file access.
    StringInfo string_info;
    {
        const size_t size = (size_hint == nullptr) ? 16384 : *size_hint;
        string_infos.reserve(size);
    }

    ReadStep step = ReadStep::Value::LABEL;
    char eol = '\n';

    while (rts::read_line(file.Get(), read8.data(), read8.size(), nullptr, eol)) {
        if (step == ReadStep::Value::LABEL) {
            if (!Try_Parse_STR_Label(read8, string_info)) {
                continue;
            }
        } else if (step == ReadStep::Value::TEXT_BEGIN) {
            Parse_STR_Text_Begin(read8, string_info);
        } else if (step == ReadStep::Value::TEXT_END) {
            Parse_STR_Text_End(read8, string_info, options);
        } else if (step == ReadStep::Value::END) {
            if (!Try_Parse_STR_End(read8, string_info)) {
                // Is not end. Continue looking for end.
                continue;
            }
            // Is end. Push to container and continue with next label.
            string_infos.push_back(string_info);
        }

        Next_Step(step, eol);
    }

    if (options.has(Options::Value::OPTIMIZE_MEMORY_SIZE)) {
        rts::shrink_to_fit(string_infos);
    }

    return !string_infos.empty();
}

bool GameTextFile::Try_Parse_STR_Label(Utf8View read8, StringInfo &string_info)
{
    rts::strip_characters(read8.data(), "\n\r");
    const size_t len = rts::strip_leading_and_trailing_spaces(read8.data());

    if (len == 0) {
        return false;
    }
    if (Is_STR_Comment(read8.data())) {
        return false;
    }
    string_info.label = read8.data();
    return true;
}

void GameTextFile::Parse_STR_Text_Begin(Utf8View read8, StringInfo &string_info)
{
    // Read string can be empty.

    rts::strip_characters(read8.data(), "\n\r");
}

void GameTextFile::Parse_STR_Text_End(Utf8View read8, StringInfo &string_info, Options options)
{
    // Read string can be empty.

    const auto escaped_chars_view = Escaped_Characters_For_STR_Read<char>();

    rts::strip_characters(read8.data(), "\n\r");
    rts::replace_characters(read8.data(), "\t\v\f", ' ');

    // STR does support escaped characters for special control characters. When written out as 2 symbol sequence, it will be
    // converted into single character here. Convert in place.
    rts::convert_from_escaped_characters(read8.data(), read8.size(), read8.data(), escaped_chars_view);

    if (!options.has(GameTextOption::KEEP_SPACES_ON_STR_READ)) {
        // Strip any remaining obsolete spaces for cleaner presentation in game.
        rts::strip_obsolete_spaces(read8.data());
    }

    // Translate final UTF16 string.
    string_info.text.Translate(read8.data());
}

bool GameTextFile::Try_Parse_STR_End(Utf8View read8, StringInfo &string_info)
{
    rts::strip_characters(read8.data(), "\n\r");
    const size_t len = rts::strip_leading_and_trailing_spaces(read8.data());

    if (len == 0) {
        return false;
    }
    if (Is_STR_Comment(read8.data())) {
        return false;
    }
    if (Is_STR_End(read8.data())) {
        return true;
    }
    string_info.speech = read8.data();
    return false;
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
    if (++step > ReadStep::Value::END) {
        step = ReadStep::Value::LABEL;
    }

    switch (step.value()) {
        case ReadStep::Value::LABEL:
            eol = '\n';
            break;
        case ReadStep::Value::TEXT_BEGIN:
            eol = '"';
            break;
        case ReadStep::Value::TEXT_END:
            eol = '"';
            break;
        case ReadStep::Value::END:
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
            language = (header.version > 1) ? static_cast<LanguageID>(header.langid) : LanguageID::US;
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

bool GameTextFile::Write_STR_Multi_File(
    FileRef &file, ConstStringInfosView string_infos_view, Languages languages, Options options)
{
    captainslog_assert(string_infos_view.size() == static_cast<size_t>(LanguageID::COUNT));

    const StringInfos *string_infos[size_t(LanguageID::COUNT)] = { nullptr };

    For_Each_Language(languages, [&](LanguageID language) {
        const size_t index = static_cast<size_t>(language);
        string_infos[index] = &string_infos_view[index];
    });

    return Write_STR_Multi_File(file, ConstStringInfosPtrView(string_infos), languages, options);
}

bool GameTextFile::Write_STR_Multi_File(
    FileRef &file, ConstStringInfosPtrView string_infos_ptrs, Languages languages, Options options)
{
    captainslog_assert(string_infos_ptrs.size() == static_cast<size_t>(LanguageID::COUNT));
    captainslog_info("Writing string file '%s' in STR multi format", file->Get_File_Name().Str());

    MultiStringInfos multi_string_infos;
    Build_Multi_String_Infos(multi_string_infos, string_infos_ptrs, options);

    char write_buf[GAMETEXT_BUFFER_8_SIZE] = { 0 };
    Utf8View w1 = write_buf;

    Utf8String w2;
    w2.Get_Buffer_For_Read(GAMETEXT_BUFFER_8_SIZE);

    for (const MultiStringInfo &string_info : multi_string_infos) {
        if (!string_info.label.Is_Empty()) {
            if (!Write_STR_Multi_Entry(file, string_info, languages, options, w1, w2)) {
                return false;
            }
        }
    }
    return true;
}

bool GameTextFile::Write_STR_Multi_Entry(
    FileRef &file, const MultiStringInfo &string_info, Languages languages, Options options, Utf8View w1, Utf8String &w2)
{
    bool ok = true;

    ok &= Write_STR_Label(file, string_info.label);

    For_Each_Language(languages, [&](LanguageID language) {
        const size_t index = static_cast<size_t>(language);

        ok &= Write_STR_Language(file, language);
        ok &= Write_STR_Text(file, string_info.text[index], options, w1, w2);

        if (!string_info.speech[index].Is_Empty()) {
            ok &= Write_STR_Language(file, language);
            ok &= Write_STR_Speech(file, string_info.speech[index]);
        }
    });

    ok &= Write_STR_End(file);

    return ok;
}

bool GameTextFile::Write_STR_Language(FileRef &file, LanguageID language)
{
    const char *code = Get_Language_Code(language);

    bool ok = true;
    ok &= rts::write_str(file.Get(), rts::make_array_view(code));
    ok &= rts::write_any(file.Get(), s_str_lng);
    return ok;
}

bool GameTextFile::Write_STR_File(FileRef &file, const StringInfos &string_infos, Options options)
{
    captainslog_info("Writing string file '%s' in STR format", file->Get_File_Name().Str());

    char write_buf[GAMETEXT_BUFFER_8_SIZE] = { 0 };
    Utf8View w1 = write_buf;

    Utf8String w2;
    w2.Get_Buffer_For_Read(GAMETEXT_BUFFER_8_SIZE);

    for (const StringInfo &string_info : string_infos) {
        if (!string_info.label.Is_Empty()) {
            if (!Write_STR_Entry(file, string_info, options, w1, w2)) {
                return false;
            }
        }
    }
    return true;
}

bool GameTextFile::Write_STR_Entry(
    FileRef &file, const StringInfo &string_info, Options options, Utf8View w1, Utf8String &w2)
{
    bool ok = true;
    ok &= Write_STR_Label(file, string_info.label);
    ok &= Write_STR_Text(file, string_info.text, options, w1, w2);

    if (!string_info.speech.Is_Empty()) {
        ok &= Write_STR_Speech(file, string_info.speech);
    }
    ok &= Write_STR_End(file);
    return ok;
}

bool GameTextFile::Write_STR_Label(FileRef &file, const Utf8String &label)
{
    bool ok = true;
    ok &= rts::write_str(file.Get(), rts::make_array_view(label));
    ok &= rts::write_any(file.Get(), s_str_eol);
    return ok;
}

bool GameTextFile::Write_STR_Text(FileRef &file, const Utf16String &text, Options options, Utf8View w1, Utf8String &w2)
{
    // Convert utf16 to utf8.
    w2.Translate(text.Str());

    size_t len;

    // STR does support escaped characters for special control characters. Write them out as escaped characters so they are
    // easily modifiable in text editor.
    {
        const auto escaped_chars_view = Escaped_Characters_For_STR_Write<char>();
        len = rts::convert_to_escaped_characters(w1.data(), w1.size(), w2.Str(), escaped_chars_view);
    }

    if (options.has(Options::Value::PRINT_LINEBREAKS_ON_STR_WRITE)) {
        // Add CR LF characters behind each written out line feed for better readability. These characters will be ignored
        // when read back from the STR file.
        w2 = w1.data();
        const char *search = "\\n";
        const char *rplace = "\\n\r\n";
        len = rts::replace_character_sequence(w1.data(), w1.size(), w2.Str(), search, rplace);
    }

    bool ok = true;
    ok &= rts::write_any(file.Get(), s_str_quo);
    ok &= rts::write_str(file.Get(), rts::make_array_view(w1.data(), len));
    ok &= rts::write_any(file.Get(), s_str_quo);
    ok &= rts::write_any(file.Get(), s_str_eol);
    return ok;
}

bool GameTextFile::Write_STR_Speech(FileRef &file, const Utf8String &speech)
{
    bool ok = true;
    ok &= rts::write_str(file.Get(), rts::make_array_view(speech));
    ok &= rts::write_any(file.Get(), s_str_eol);
    return ok;
}

bool GameTextFile::Write_STR_End(FileRef &file)
{
    bool ok = true;
    ok &= rts::write_any(file.Get(), s_str_end);
    ok &= rts::write_any(file.Get(), s_str_eol);
    ok &= rts::write_any(file.Get(), s_str_eol);
    return ok;
}

bool GameTextFile::Write_CSF_File(FileRef &file, const StringInfos &string_infos, const LanguageID &language)
{
    captainslog_info("Writing string file '%s' in CSF format", file->Get_File_Name().Str());

    bool success = false;

    if (Write_CSF_Header(file, string_infos, language)) {
        success = true;
        unichar_t write_buf[GAMETEXT_BUFFER_16_SIZE] = { 0 };
        Utf16View write16 = write_buf;
        int string_index = 0;

        for (const StringInfo &string_info : string_infos) {
            ++string_index;

            if (string_info.label.Is_Empty()) {
                captainslog_error("String %d has no label", string_index);
                continue;
            }

            if (!Write_CSF_Entry(file, string_info, write16)) {
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

bool GameTextFile::Write_CSF_Entry(FileRef &file, const StringInfo &string_info, Utf16View write16)
{
    if (Write_CSF_Label(file, string_info)) {
        if (Write_CSF_Text(file, string_info, write16)) {
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

bool GameTextFile::Write_CSF_Text(FileRef &file, const StringInfo &string_info, Utf16View write16)
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
            captainslog_dbgassert(write16.size() >= text_len, "Buffer is expected to be larger or equal text");
            text_len = std::min(text_len, write16.size());

            for (size_t i = 0; i < text_len; ++i) {
                // Every char is binary flipped here by design.
                write16[i] = ~string_info.text[i];
                htole_ref(write16[i]);
            }

            if (rts::write(file.Get(), write16.data(), text_len * sizeof(Utf16View::value_type))) {
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
