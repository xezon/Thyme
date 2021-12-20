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
#include "utility/arrayutil.h"
#include "utility/fileutil.h"
#include "utility/stlutil.h"
#include "utility/stringutil.h"

namespace Thyme
{

namespace
{
template<typename IntegerType> constexpr size_t Bit_To_Index(IntegerType integer)
{
    using UnsignedIntegerType = rts::unsigned_integer<IntegerType>::type;
    size_t n = 0;
    for (; n < sizeof(IntegerType) * 8; ++n) {
        if (UnsignedIntegerType(integer) & (UnsignedIntegerType(1) << n)) {
            return n;
        }
    }
    return n;
}

constexpr const char *const s_option_0 = "None";
constexpr const char *const s_option_1 = "Check_Buffer_Length_On_Load";
constexpr const char *const s_option_2 = "Check_Buffer_Length_On_Save";
constexpr const char *const s_option_3 = "Keep_Spaces_On_STR_Load";
constexpr const char *const s_option_4 = "Print_Linebreaks_On_STR_Save";
constexpr const char *const s_option_5 = "Optimize_Memory_Size";

constexpr const char *const s_options[] = {
    s_option_0,
    s_option_1,
    s_option_2,
    s_option_3,
    s_option_4,
    s_option_5,
};

static_assert(s_option_0 == s_options[size_t(GameTextOption::NONE)], "Error");
static_assert(s_option_1 == s_options[1 + Bit_To_Index(GameTextOption::CHECK_BUFFER_LENGTH_ON_LOAD)], "Error");
static_assert(s_option_2 == s_options[1 + Bit_To_Index(GameTextOption::CHECK_BUFFER_LENGTH_ON_SAVE)], "Error");
static_assert(s_option_3 == s_options[1 + Bit_To_Index(GameTextOption::KEEP_SPACES_ON_STR_LOAD)], "Error");
static_assert(s_option_4 == s_options[1 + Bit_To_Index(GameTextOption::PRINT_LINEBREAKS_ON_STR_SAVE)], "Error");
static_assert(s_option_5 == s_options[1 + Bit_To_Index(GameTextOption::OPTIMIZE_MEMORY_SIZE)], "Error");
} // namespace

bool Name_To_Game_Text_Option(const char *name, GameTextOption &option)
{
    size_t index = 0;
    for (const char *option_name : s_options) {
        if (strcasecmp(option_name, name) == 0) {
            option = (index == 0) ? GameTextOption::NONE : static_cast<GameTextOption>(1 << (index - 1));
            return true;
        }
        ++index;
    }
    return false;
}

namespace
{
template<typename CharType> rts::escaped_char_alias_view<CharType> Escaped_Characters_For_STR_Read()
{
    static CHAR_TRAITS_CONSTEXPR rts::escaped_char_alias<CharType> escaped_chars[] = {
        rts::escaped_char_alias<CharType>::Make_Real_Alias2('\n', '\\', 'n'),
        rts::escaped_char_alias<CharType>::Make_Real_Alias2('\t', '\\', 't'),
        rts::escaped_char_alias<CharType>::Make_Real_Alias2('"', '\\', '"'),
        rts::escaped_char_alias<CharType>::Make_Real_Alias2('?', '\\', '?'),
        rts::escaped_char_alias<CharType>::Make_Real_Alias2('\'', '\\', '\''),
        rts::escaped_char_alias<CharType>::Make_Real_Alias2('\\', '\\', '\\'),
    };
    return rts::escaped_char_alias_view<CharType>(escaped_chars);
}

template<typename CharType> rts::escaped_char_alias_view<CharType> Escaped_Characters_For_STR_Write()
{
    static CHAR_TRAITS_CONSTEXPR rts::escaped_char_alias<CharType> escaped_chars[] = {
        rts::escaped_char_alias<CharType>::Make_Real_Alias2('\n', '\\', 'n'),
        rts::escaped_char_alias<CharType>::Make_Real_Alias2('\t', '\\', 't'),
        rts::escaped_char_alias<CharType>::Make_Real_Alias2('"', '\\', '"'),
        rts::escaped_char_alias<CharType>::Make_Real_Alias2('\\', '\\', '\\'),
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
constexpr const char *const s_langcode___ = "__";
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

static_assert(s_langcode_us == s_langcodes[size_t(LanguageID::US)], "Error");
static_assert(s_langcode_en == s_langcodes[size_t(LanguageID::UK)], "Error");
static_assert(s_langcode_de == s_langcodes[size_t(LanguageID::GERMAN)], "Error");
static_assert(s_langcode_fr == s_langcodes[size_t(LanguageID::FRENCH)], "Error");
static_assert(s_langcode_es == s_langcodes[size_t(LanguageID::SPANISH)], "Error");
static_assert(s_langcode_it == s_langcodes[size_t(LanguageID::ITALIAN)], "Error");
static_assert(s_langcode_ja == s_langcodes[size_t(LanguageID::JAPANESE)], "Error");
static_assert(s_langcode_jb == s_langcodes[size_t(LanguageID::JABBER)], "Error");
static_assert(s_langcode_ko == s_langcodes[size_t(LanguageID::KOREAN)], "Error");
static_assert(s_langcode_zh == s_langcodes[size_t(LanguageID::CHINESE)], "Error");
static_assert(s_langcode___ == s_langcodes[size_t(LanguageID::UNUSED_1)], "Error");
static_assert(s_langcode_bp == s_langcodes[size_t(LanguageID::BRAZILIAN)], "Error");
static_assert(s_langcode_pl == s_langcodes[size_t(LanguageID::POLISH)], "Error");
static_assert(s_langcode___ == s_langcodes[size_t(LanguageID::UNKNOWN)], "Error");
static_assert(s_langcode_ru == s_langcodes[size_t(LanguageID::RUSSIAN)], "Error");
static_assert(s_langcode_ar == s_langcodes[size_t(LanguageID::ARABIC)], "Error");

static_assert(ARRAY_SIZE(s_langcodes) == LanguageCount, "Error");

constexpr const char *Get_Language_Code(LanguageID language)
{
    return s_langcodes[size_t(language)];
}

constexpr const char s_str_eol[] = { '\r', '\n' };
constexpr const char s_str_quo[] = { '"' };
constexpr const char s_str_end[] = { 'E', 'N', 'D' };
constexpr const char s_str_lng[] = { ':' };

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

bool GameTextFile::Is_Any_Loaded(Languages languages) const
{
    bool loaded = true;
    For_Each_Language(languages, [&](LanguageID language) { loaded |= !Get_String_Infos(language).empty(); });
    return loaded;
}

bool GameTextFile::Load(const char *filename)
{
    Type filetype = Get_File_Type(filename, Type::AUTO);
    return Load(filename, filetype, nullptr);
}

bool GameTextFile::Load_CSF(const char *filename)
{
    return Load(filename, Type::CSF, nullptr);
}

bool GameTextFile::Load_STR(const char *filename)
{
    return Load(filename, Type::STR, nullptr);
}

bool GameTextFile::Load_STR(const char *filename, Languages languages)
{
    return Load(filename, Type::STR, &languages);
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

bool GameTextFile::Save_STR(const char *filename)
{
    return Save(filename, Type::STR, nullptr);
}

bool GameTextFile::Save_STR(const char *filename, Languages languages)
{
    return Save(filename, Type::STR, &languages);
}

bool GameTextFile::Load(const char *filename, Type filetype, const Languages *languages)
{
    captainslog_assert(filetype != Type::AUTO);

    if (!filename || !filename[0]) {
        captainslog_error("File without name cannot be loaded");
        return false;
    }

    FileRef file = g_theFileSystem->Open(filename, File::READ | File::BINARY);
    if (!file.Is_Open()) {
        captainslog_error("File '%s' cannot be opened for read", filename);
        return false;
    }

    bool success = false;

    LanguageID read_language = m_language;
    StringInfosArray string_infos_array;
    StringInfos &string_infos = string_infos_array[size_t(read_language)];

    switch (filetype) {

        case Type::CSF: {
            success = Read_CSF_File(file, string_infos, read_language);
            string_infos_array[size_t(read_language)].swap(string_infos);
            break;
        }
        case Type::STR: {
            if (languages != nullptr) {
                StringInfosPtrArray string_infos_ptrs = Build_String_Infos_Ptrs_Array(string_infos_array, *languages);
                success = Read_Multi_STR_File(file, string_infos_ptrs, *languages, m_options);
                Get_Language_With_String_Infos(read_language, string_infos_ptrs, 0);
            } else {
                success = Read_STR_File(file, string_infos, m_options);
            }
            break;
        }
    }

    if (success) {
        m_language = read_language;
        const Languages used_languages = (languages == nullptr) ? read_language : *languages;

        captainslog_info("File '%s' loaded successfully", filename);

        For_Each_Language(used_languages, [&](LanguageID language) {
            Mutable_String_Infos(language).swap(string_infos_array[size_t(language)]);

            captainslog_info("Read language: %s", Get_Language_Name(language));
            captainslog_info("Read line count: %zu", Get_String_Infos(language).size());

            if (m_options.has(Options::Value::CHECK_BUFFER_LENGTH_ON_LOAD)) {
                Check_Buffer_Lengths(language);
            }
        });
    } else {
        captainslog_info("File '%s' failed to load", filename);
    }

    return success;
}

bool GameTextFile::Save(const char *filename, Type filetype, const Languages *languages)
{
    captainslog_assert(filetype != Type::AUTO);

    if (!filename || !filename[0]) {
        captainslog_error("File without name cannot be saved");
        return false;
    }

    const Languages used_languages = (languages == nullptr) ? m_language : *languages;

    if (!Is_Any_Loaded(used_languages)) {
        captainslog_error("File without string data cannot be saved");
        return false;
    }

    FileRef file = g_theFileSystem->Open(filename, File::WRITE | File::CREATE | File::BINARY);
    if (!file.Is_Open()) {
        captainslog_error("File '%s' cannot be opened for write", filename);
        return false;
    }

    bool success = false;

    switch (filetype) {

        case Type::CSF: {
            success = Write_CSF_File(file, Get_String_Infos(), m_language);
            break;
        }
        case Type::STR: {
            if (languages != nullptr) {
                ConstStringInfosPtrArray string_infos_ptrs =
                    Build_Const_String_Infos_Ptrs_Array(m_stringInfosArray, *languages);
                success = Write_Multi_STR_File(file, string_infos_ptrs, *languages, m_options);
            } else {
                success = Write_STR_File(file, Get_String_Infos(), m_options);
            }
            break;
        }
    }

    if (success) {
        captainslog_info("File '%s' saved successfully", filename);

        For_Each_Language(used_languages, [&](LanguageID language) {
            captainslog_info("Written language: %s", Get_Language_Name(language));
            captainslog_info("Written line count: %zu", Get_String_Infos(language).size());

            if (m_options.has(Options::Value::CHECK_BUFFER_LENGTH_ON_SAVE)) {
                Check_Buffer_Lengths(language);
            }
        });
    } else {
        captainslog_info("File '%s' failed to save", filename);
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
    for (StringInfos &string_infos : m_stringInfosArray) {
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

void GameTextFile::Check_Buffer_Lengths(LanguageID language)
{
    LengthInfo len_info;
    Collect_Length_Info(len_info, Get_String_Infos(language));
    Log_Length_Info(len_info);
    Assert_Length_Info(len_info);
}

const StringInfos &GameTextFile::Get_String_Infos() const
{
    return m_stringInfosArray[size_t(m_language)];
}

const StringInfos &GameTextFile::Get_String_Infos(LanguageID language) const
{
    return m_stringInfosArray[size_t(language)];
}

StringInfos &GameTextFile::Mutable_String_Infos()
{
    return m_stringInfosArray[size_t(m_language)];
}

StringInfos &GameTextFile::Mutable_String_Infos(LanguageID language)
{
    return m_stringInfosArray[size_t(language)];
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
        m_stringInfosArray[size_t(left)].swap(m_stringInfosArray[size_t(right)]);
    }
}

template<typename Functor> static void GameTextFile::For_Each_Language(Languages languages, Functor functor)
{
    for (rts::enumerator<LanguageID> it; it < LanguageID(LanguageCount); ++it) {
        const LanguageID language = it.value();
        if (languages.has(language)) {
            functor(language);
        }
    }
}

GameTextFile::StringInfosPtrArray GameTextFile::Build_String_Infos_Ptrs_Array(
    StringInfosArray &string_infos_array, Languages languages)
{
    StringInfosPtrArray string_infos_ptrs = {};

    For_Each_Language(languages, [&](LanguageID language) {
        const size_t index = static_cast<size_t>(language);
        string_infos_ptrs[index] = &string_infos_array[index];
    });
    return string_infos_ptrs;
}

GameTextFile::ConstStringInfosPtrArray GameTextFile::Build_Const_String_Infos_Ptrs_Array(
    StringInfosArray &string_infos_array, Languages languages)
{
    ConstStringInfosPtrArray string_infos_ptrs = {};

    For_Each_Language(languages, [&](LanguageID language) {
        const size_t index = static_cast<size_t>(language);
        string_infos_ptrs[index] = &string_infos_array[index];
    });
    return string_infos_ptrs;
}

size_t GameTextFile::Get_Max_Size(const ConstStringInfosPtrArray &string_infos_ptrs)
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
    MultiStringInfos &multi_string_infos, const ConstStringInfosPtrArray &string_infos_ptrs, Options options)
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
    StringInfosPtrArray &string_infos_ptrs, const MultiStringInfos &multi_string_infos, Options options)
{
    size_t language_index = 0;

    for (StringInfos *string_infos_ptr : string_infos_ptrs) {
        if (string_infos_ptr != nullptr) {
            StringInfos &string_infos = *string_infos_ptr;
            if (options.has(Options::Value::OPTIMIZE_MEMORY_SIZE)) {
                rts::free_container(string_infos);
            }
            string_infos.resize(multi_string_infos.size());
            size_t string_index = 0;

            for (const MultiStringInfo &multi_string_info : multi_string_infos) {
                string_infos[string_index].label = multi_string_info.label;
                string_infos[string_index].text = multi_string_info.text[language_index];
                string_infos[string_index].speech = multi_string_info.speech[language_index];
                ++string_index;
            }
        }
        ++language_index;
    }
}

bool GameTextFile::Get_Language_With_String_Infos(
    LanguageID &language, const StringInfosPtrArray &string_infos_ptrs, size_t occurence)
{
    size_t num = 0;
    rts::enumerator<LanguageID> it;
    for (const StringInfosPtrArray::value_type &string_infos_ptr : string_infos_ptrs) {
        if (string_infos_ptr != nullptr && !string_infos_ptr->empty()) {
            if (occurence == num++) {
                language = it.value();
                return true;
            }
        }
        ++it;
    }
    return false;
}

GameTextFile::Type GameTextFile::Get_File_Type(const char *filename, Type filetype)
{
    if (filetype == Type::AUTO) {
        const char *fileext = rts::Get_File_Extension(rts::Make_Array_View(filename));
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
    len_info.max_text8_len = 0;
    len_info.max_text16_len = 0;
    len_info.max_speech_len = 0;

    Utf8String utf8text;

    for (const StringInfo &string : strings) {
        utf8text.Translate(string.text);

        len_info.max_label_len = std::max(len_info.max_label_len, string.label.Get_Length());
        len_info.max_text8_len = std::max(len_info.max_text8_len, utf8text.Get_Length());
        len_info.max_text16_len = std::max(len_info.max_text16_len, string.text.Get_Length());
        len_info.max_speech_len = std::max(len_info.max_speech_len, string.speech.Get_Length());
    }
}

void GameTextFile::Log_Length_Info(const LengthInfo &len_info)
{
    const int label_len = len_info.max_label_len;
    const int text8_len = len_info.max_text8_len;
    const int text16_len = len_info.max_text16_len;
    const int speech_len = len_info.max_speech_len;

    const int label_err = (TEXT_8_SIZE - 1 > label_len) ? LOGLEVEL_INFO : LOGLEVEL_ERROR;
    const int text8_err = (TEXT_8_SIZE - 1 > text8_len) ? LOGLEVEL_INFO : LOGLEVEL_ERROR;
    const int text16_err = (TEXT_16_SIZE - 1 > text16_len) ? LOGLEVEL_INFO : LOGLEVEL_ERROR;
    const int speech_err = (TEXT_8_SIZE - 1 > speech_len) ? LOGLEVEL_INFO : LOGLEVEL_ERROR;

    captainslog_log(label_err, __FILE__, __LINE__, "Checked label len: %d, max: %d", label_len, TEXT_8_SIZE);
    captainslog_log(text8_err, __FILE__, __LINE__, "Checked utf8 text len: %d, max: %d", text8_len, TEXT_8_SIZE);
    captainslog_log(text16_err, __FILE__, __LINE__, "Checked utf16 text len: %d, max: %d", text16_len, TEXT_16_SIZE);
    captainslog_log(speech_err, __FILE__, __LINE__, "Checked speech len: %d, max: %d", speech_len, TEXT_8_SIZE);
}

void GameTextFile::Assert_Length_Info(const LengthInfo &len_info)
{
    captainslog_dbgassert(TEXT_8_SIZE - 1 > len_info.max_label_len, "Buffer size must be larger");
    captainslog_dbgassert(TEXT_8_SIZE - 1 > len_info.max_text8_len, "Buffer size must be larger");
    captainslog_dbgassert(TEXT_16_SIZE - 1 > len_info.max_text16_len, "Buffer size must be larger");
    captainslog_dbgassert(TEXT_8_SIZE - 1 > len_info.max_speech_len, "Buffer size must be larger");
}

Utf16String &GameTextFile::Get_Text(StringInfo &string_info, LanguageID language)
{
    (void)language;
    return string_info.text;
}

Utf16String &GameTextFile::Get_Text(MultiStringInfo &string_info, LanguageID language)
{
    return string_info.text[size_t(language)];
}

Utf8String &GameTextFile::Get_Speech(StringInfo &string_info, LanguageID language)
{
    (void)language;
    return string_info.speech;
}

Utf8String &GameTextFile::Get_Speech(MultiStringInfo &string_info, LanguageID language)
{
    return string_info.speech[size_t(language)];
}

bool GameTextFile::Read_Multi_STR_File(
    FileRef &file, StringInfosPtrArray &string_infos_ptrs, Languages languages, Options options)
{
    captainslog_info("Reading text file '%s' in STR multi format", file->Get_File_Name().Str());

    MultiStringInfos multi_string_infos;
    multi_string_infos.reserve(8192);

    Read_STR_File_T(file, multi_string_infos, options);

    Build_String_Infos(string_infos_ptrs, multi_string_infos, options);

    return !multi_string_infos.empty();
}

bool GameTextFile::Read_STR_File(FileRef &file, StringInfos &string_infos, Options options)
{
    captainslog_info("Reading text file '%s' in STR format", file->Get_File_Name().Str());

    // Instead of reading the file once from top to bottom to get the number of the entries, we will allocate a very generous
    // buffer to begin with and shrink it down afterwards. This will reduce algorithm complexity and file access.
    string_infos.reserve(8192);

    Read_STR_File_T(file, string_infos, options);

    if (options.has(Options::Value::OPTIMIZE_MEMORY_SIZE)) {
        rts::shrink_to_fit(string_infos);
    }

    return !string_infos.empty();
}

template<typename StringInfosType>
static void GameTextFile::Read_STR_File_T(FileRef &file, StringInfosType &string_infos, Options options)
{
    using StringInfoType = typename StringInfosType::value_type;

    StringInfoType string_info;
    StrParseResult result;
    LanguageID read_language = LanguageID::UNKNOWN;
    Utf8Array read = {};
    StrReadStep step = StrReadStep::LABEL;
    const char *eol_chars = "\n";

    while (rts::Read_Line(file.Get(), read.data(), read.size(), eol_chars)) {

        switch (step) {
            case StrReadStep::LABEL:
                string_info = StringInfoType();
                result = Parse_STR_Label(read, string_info.label);

                if (result == StrParseResult::IS_LABEL) {
                    Change_Step(step, StrReadStep::SEARCH, eol_chars);
                }
                break;

            case StrReadStep::SEARCH:
                read_language = LanguageID::UNKNOWN;
                result = Parse_STR_Search(read);

                if (result == StrParseResult::IS_PRETEXT) {
                    size_t parsed_count;
                    Parse_STR_Language(read.data(), read_language, parsed_count);
                    Change_Step(step, StrReadStep::TEXT, eol_chars);

                } else if (result == StrParseResult::IS_SPEECH) {
                    size_t parsed_count;
                    Parse_STR_Language(read.data(), read_language, parsed_count);
                    Utf8View view(read.data() + parsed_count, read.size() - parsed_count);
                    Parse_STR_Speech(view, Get_Speech(string_info, read_language));

                } else if (result == StrParseResult::IS_END) {
                    string_infos.push_back(string_info);
                    Change_Step(step, StrReadStep::LABEL, eol_chars);
                }
                break;

            case StrReadStep::TEXT:
                Parse_STR_Text(read, Get_Text(string_info, read_language), options);
                Change_Step(step, StrReadStep::SEARCH, eol_chars);
                break;
        }
    }
}

GameTextFile::StrParseResult GameTextFile::Parse_STR_Label(Utf8Array &read, Utf8String &label)
{
    rts::Strip_Characters(read.data(), "\n\r");
    const size_t len = rts::Strip_Leading_And_Trailing_Spaces(read.data());

    if (len == 0) {
        return StrParseResult::IS_NOTHING;
    }

    if (Is_STR_Comment(read.data())) {
        return StrParseResult::IS_NOTHING;
    }

    label = read.data();
    return StrParseResult::IS_LABEL;
}

GameTextFile::StrParseResult GameTextFile::Parse_STR_Search(Utf8Array &read)
{
    rts::Strip_Characters(read.data(), "\n\r");
    const size_t len = rts::Strip_Leading_And_Trailing_Spaces(read.data());

    if (len == 0) {
        return StrParseResult::IS_NOTHING;
    }

    if (Is_STR_Comment(read.data())) {
        return StrParseResult::IS_NOTHING;
    }

    if (Is_STR_End(read.data())) {
        return StrParseResult::IS_END;
    }

    Utf8View view(read.begin(), len);

    // #TODO the original appears to have some more checks for the Speech string. Could include them here as well.

    return Is_STR_Pre_Text(view) ? StrParseResult::IS_PRETEXT : StrParseResult::IS_SPEECH;
}

void GameTextFile::Parse_STR_Text(Utf8Array &read, Utf16String &text, Options options)
{
    // Read string can be empty.

    const auto escaped_chars_view = Escaped_Characters_For_STR_Read<char>();

    rts::Strip_Characters(read.data(), "\n\r");
    rts::Replace_Characters(read.data(), "\t\v\f", ' ');

    // STR does support escaped characters for special control characters. When written out as 2 symbol sequence, it will be
    // converted into single character here. Convert in place.
    size_t len = rts::Convert_From_Escaped_Characters(read.data(), read.size(), read.data(), escaped_chars_view);

    // Read string is expected to close with a quote. Remove it here.
    if (read[len - 1] == '\"') {
        read[len - 1] = '\0';
    }

    if (!options.has(GameTextOption::KEEP_SPACES_ON_STR_LOAD)) {
        // Strip any remaining obsolete spaces for cleaner presentation in game.
        rts::Strip_Obsolete_Spaces(read.data());
    }

    // Translate final UTF16 string.
    text.Translate(read.data());
}

void GameTextFile::Parse_STR_Speech(Utf8View &read, Utf8String &speech)
{
    rts::Strip_Characters(read.data(), "\n\r");
    rts::Strip_Leading_And_Trailing_Spaces(read.data());
    speech = read.data();
}

bool GameTextFile::Parse_STR_Language(const char *cstring, LanguageID &language, size_t &parsed_count)
{
    const size_t code_len = strlen(s_langcodes[0]);
    const size_t lng_len = strlen(s_str_lng);

    for (rts::enumerator<LanguageID> it; it < LanguageID(LanguageCount); ++it) {
        const char *code = s_langcodes[it.underlying()];
        if (strncasecmp(cstring, code, code_len) == 0) {
            if (strncasecmp(cstring + code_len, s_str_lng, lng_len) == 0) {
                language = it.value();
                parsed_count = code_len + lng_len;
                return true;
            }
        }
    }
    parsed_count = 0;
    return false;
}

bool GameTextFile::Is_STR_Pre_Text(Utf8View read)
{
    return !read.empty() ? (read.back() == '"') : false;
}

bool GameTextFile::Is_STR_Comment(const char *cstring)
{
    return (cstring[0] == '\\' && cstring[1] == '\\');
}

bool GameTextFile::Is_STR_End(const char *cstring)
{
    return (strcasecmp(cstring, "END") == 0);
}

void GameTextFile::Change_Step(StrReadStep &step, StrReadStep new_step, const char *&eol_chars)
{
    step = new_step;

    switch (new_step) {
        case StrReadStep::LABEL:
            eol_chars = "\n";
            break;
        case StrReadStep::SEARCH:
            eol_chars = "\n\"";
            break;
        case StrReadStep::TEXT:
            eol_chars = "\"";
            break;
    }
}

bool GameTextFile::Read_CSF_File(FileRef &file, StringInfos &string_infos, LanguageID &language)
{
    captainslog_info("Reading text file '%s' in CSF format", file->Get_File_Name().Str());

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

    if (rts::Read_Any(file.Get(), header)) {
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

    if (rts::Read_Any(file.Get(), header)) {
        letoh_ref(header.id);
        letoh_ref(header.texts);
        letoh_ref(header.length);

        if (header.id == rts::FourCC_LE<'L', 'B', 'L', ' '>::value) {
            if (rts::Read_Str(file.Get(), rts::Make_Resized_Array_View(string_info.label, header.length))) {
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

        if (rts::Read_Any(file.Get(), header)) {
            letoh_ref(header.id);
            letoh_ref(header.length);

            read_speech = (header.id == rts::FourCC_LE<'S', 'T', 'R', 'W'>::value);
            const bool read_text = (header.id == rts::FourCC_LE<'S', 'T', 'R', ' '>::value);

            if (read_speech || read_text) {
                auto str = rts::Make_Resized_Array_View(string_info.text, header.length);

                if (rts::Read_Str(file.Get(), str)) {
                    for (int32_t i = 0; i < header.length; ++i) {
                        letoh_ref(string_info.text[i]);
                        // Every char is binary flipped here by design.
                        string_info.text[i] = ~string_info.text[i];
                    }

                    if (str.data() != nullptr) {
                        // Strip obsolete spaces for cleaner presentation in game.
                        rts::Strip_Obsolete_Spaces(str.data());
                    }

                    text_ok = true;
                }
            }
        }
    }

    if (read_speech) {
        CSFSpeechHeader header;

        if (rts::Read_Any(file.Get(), header)) {
            letoh_ref(header.length);

            if (rts::Read_Str(file.Get(), rts::Make_Resized_Array_View(string_info.speech, header.length))) {
                speech_ok = true;
            }
        }
    }

    return text_ok && (speech_ok || !read_speech);
}

bool GameTextFile::Write_Multi_STR_File(
    FileRef &file, const ConstStringInfosPtrArray &string_infos_ptrs, Languages languages, Options options)
{
    captainslog_info("Writing text file '%s' in STR multi format", file->Get_File_Name().Str());

    MultiStringInfos multi_string_infos;
    Build_Multi_String_Infos(multi_string_infos, string_infos_ptrs, options);

    Utf8Array w1 = {};
    Utf8String w2;
    w2.Get_Buffer_For_Read(TEXT_8_SIZE);

    for (const MultiStringInfo &string_info : multi_string_infos) {
        if (!string_info.label.Is_Empty()) {
            if (!Write_Multi_STR_Entry(file, string_info, languages, options, w1, w2)) {
                return false;
            }
        }
    }
    return true;
}

bool GameTextFile::Write_Multi_STR_Entry(
    FileRef &file, const MultiStringInfo &string_info, Languages languages, Options options, Utf8Array &w1, Utf8String &w2)
{
    bool ok = true;

    ok &= Write_STR_Label(file, string_info.label);

    For_Each_Language(languages, [&](LanguageID language) {
        const size_t index = static_cast<size_t>(language);
        ok &= Write_STR_Language(file, language);
        ok &= Write_STR_Text(file, string_info.text[index], options, w1, w2);
    });

    For_Each_Language(languages, [&](LanguageID language) {
        const size_t index = static_cast<size_t>(language);
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
    ok &= rts::Write_Str(file.Get(), rts::Make_Array_View(code));
    ok &= rts::Write_Any(file.Get(), s_str_lng);
    ok &= rts::Write_Any(file.Get(), ' ');
    return ok;
}

bool GameTextFile::Write_STR_File(FileRef &file, const StringInfos &string_infos, Options options)
{
    captainslog_info("Writing text file '%s' in STR format", file->Get_File_Name().Str());

    Utf8Array w1 = {};
    Utf8String w2;
    w2.Get_Buffer_For_Read(TEXT_8_SIZE);

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
    FileRef &file, const StringInfo &string_info, Options options, Utf8Array &w1, Utf8String &w2)
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
    ok &= rts::Write_Str(file.Get(), rts::Make_Array_View(label));
    ok &= rts::Write_Any(file.Get(), s_str_eol);
    return ok;
}

bool GameTextFile::Write_STR_Text(FileRef &file, const Utf16String &text, Options options, Utf8Array &w1, Utf8String &w2)
{
    // Convert utf16 to utf8.
    w2.Translate(text.Str());

    size_t len;

    // STR does support escaped characters for special control characters. Write them out as escaped characters so they are
    // easily modifiable in text editor.
    {
        const auto escaped_chars_view = Escaped_Characters_For_STR_Write<char>();
        len = rts::Convert_To_Escaped_Characters(w1.data(), w1.size(), w2.Str(), escaped_chars_view);
    }

    if (options.has(Options::Value::PRINT_LINEBREAKS_ON_STR_SAVE)) {
        // Add CR LF characters behind each written out line feed for better readability. These characters will be ignored
        // when read back from the STR file.
        w2 = w1.data();
        const char *search = "\\n";
        const char *replace = "\\n\r\n";
        len = rts::Replace_Characters_Sequence(w1.data(), w1.size(), w2.Str(), search, replace);
    }

    bool ok = true;
    ok &= rts::Write_Any(file.Get(), s_str_quo);
    ok &= rts::Write_Str(file.Get(), rts::Make_Array_View(w1.data(), len));
    ok &= rts::Write_Any(file.Get(), s_str_quo);
    ok &= rts::Write_Any(file.Get(), s_str_eol);
    return ok;
}

bool GameTextFile::Write_STR_Speech(FileRef &file, const Utf8String &speech)
{
    bool ok = true;
    ok &= rts::Write_Str(file.Get(), rts::Make_Array_View(speech));
    ok &= rts::Write_Any(file.Get(), s_str_eol);
    return ok;
}

bool GameTextFile::Write_STR_End(FileRef &file)
{
    bool ok = true;
    ok &= rts::Write_Any(file.Get(), s_str_end);
    ok &= rts::Write_Any(file.Get(), s_str_eol);
    ok &= rts::Write_Any(file.Get(), s_str_eol);
    return ok;
}

bool GameTextFile::Write_CSF_File(FileRef &file, const StringInfos &string_infos, const LanguageID &language)
{
    captainslog_info("Writing text file '%s' in CSF format", file->Get_File_Name().Str());

    bool success = false;

    if (Write_CSF_Header(file, string_infos, language)) {
        success = true;
        Utf16Array write16 = {};
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

    return rts::Write_Any(file.Get(), header);
}

bool GameTextFile::Write_CSF_Entry(FileRef &file, const StringInfo &string_info, Utf16Array &write16)
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

    if (rts::Write_Any(file.Get(), header)) {
        if (rts::Write_Str(file.Get(), rts::Make_Array_View(string_info.label))) {
            return true;
        }
    }
    return false;
}

bool GameTextFile::Write_CSF_Text(FileRef &file, const StringInfo &string_info, Utf16Array &write16)
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

        if (rts::Write_Any(file.Get(), header)) {
            captainslog_dbgassert(write16.size() >= text_len, "Buffer is expected to be larger or equal text");
            text_len = std::min(text_len, write16.size());

            for (size_t i = 0; i < text_len; ++i) {
                // Every char is binary flipped here by design.
                write16[i] = ~string_info.text[i];
                htole_ref(write16[i]);
            }

            if (rts::Write(file.Get(), write16.data(), text_len * sizeof(Utf16View::value_type))) {
                text_ok = true;
            }
        }
    }

    if (text_ok && write_speech) {
        CSFSpeechHeader header;
        header.length = string_info.speech.Get_Length();
        htole_ref(header.length);

        if (rts::Write_Any(file.Get(), header)) {
            if (rts::Write_Str(file.Get(), rts::Make_Array_View(string_info.speech))) {
                speech_ok = true;
            }
        }
    }
    return text_ok && (speech_ok || !write_speech);
}

} // namespace Thyme
