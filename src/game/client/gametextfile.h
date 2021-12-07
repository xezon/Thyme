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
#pragma once

#include "common/utility/array.h"
#include "common/utility/arrayview.h"
#include "common/utility/enumerator.h"
#include "common/utility/flags.h"
#include "fileref.h"
#include "gametextcommon.h"

namespace Thyme
{

enum class GameTextOption
{
    NONE = 0,
    CHECK_BUFFER_LENGTH_ON_LOAD = BIT(0),
    CHECK_BUFFER_LENGTH_ON_SAVE = BIT(1),
    KEEP_SPACES_ON_STR_READ = BIT(2),
    PRINT_LINEBREAKS_ON_STR_WRITE = BIT(3),
    OPTIMIZE_MEMORY_SIZE = BIT(4),
};

// #TODO Split the multi language functionality from 'GameTextFile' into a new 'MultiGameTextFile' class?

// #FEATURE GameTextFile contains the core file handling functionality of original GameTextManager, which allows to use it
// for more flexible localization file operations.
class GameTextFile
{
public:
    using Options = rts::bitflags<GameTextOption>;
    using Languages = rts::numflags<LanguageID, LanguageCount>;

    GameTextFile() :
        m_options(Options::Value::OPTIMIZE_MEMORY_SIZE), m_language(LanguageID::UNKNOWN), m_stringInfosArray(){};

    // Checks whether or not localization is loaded.
    bool Is_Loaded() const;
    bool Is_Loaded(Languages languages) const;
    bool Is_Any_Loaded(Languages languages) const;

    // Loads CSF or STR file from disk. Does not unload previous data on failure.
    bool Load(const char *filename);
    bool Load_CSF(const char *filename);
    bool Load_STR(const char *filename, const Languages *languages = nullptr);

    // Saves CSF or STR file to disk. Will write over any existing file.
    bool Save(const char *filename);
    bool Save_CSF(const char *filename);
    bool Save_STR(const char *filename, const Languages *languages = nullptr);

    // Unloads language data.
    void Unload();
    void Unload(Languages languages);

    // Unloads all data and resets all settings.
    void Reset();

    // Merges another game text into this one. Identical labels will write their text contents from the other to this.
    void Merge_And_Overwrite(const GameTextFile &other);
    void Merge_And_Overwrite(const GameTextFile &other, Languages languages);

    // Retrieves all localization data.
    const StringInfos &Get_String_Infos() const;
    const StringInfos &Get_String_Infos(LanguageID language) const;

    // Sets options for loading and saving files.
    void Set_Options(Options options);
    Options Get_Options() const;

    // Sets the active language used by this instance. Loading a CSF file will automatically change active language.
    void Set_Language(LanguageID language);
    LanguageID Get_Language() const;

    // Swaps strings from one language to another one.
    void Swap_String_Infos(LanguageID left, LanguageID right);

private:
    enum class Type
    {
        AUTO,
        CSF,
        STR,
    };

    enum class StrReadStep
    {
        LABEL,
        SEARCH,
        TEXT,
    };

    enum class StrParseResult
    {
        IS_NOTHING,
        IS_LABEL,
        IS_PRETEXT,
        IS_SPEECH,
        IS_END,
    };

    struct LengthInfo
    {
        int max_label_len;
        int max_text_utf8_len;
        int max_text_utf16_len;
        int max_speech_len;
    };

    // https://www.rfc-editor.org/rfc/rfc3629
    // In UTF-8, characters from the U+0000..U+10FFFF range (the UTF-16
    // accessible range) are encoded using sequences of 1 to 4 octets.
    enum : size_t
    {
        GAMETEXT_BUFFER_16_SIZE = 1024,
        GAMETEXT_BUFFER_8_SIZE = GAMETEXT_BUFFER_16_SIZE * 4,
    };

    using StringInfosArray = rts::array<StringInfos, LanguageCount>;
    using ConstStringInfosPtrArray = rts::array<const StringInfos *, LanguageCount>;
    using StringInfosPtrArray = rts::array<StringInfos *, LanguageCount>;
    using Utf8Array = rts::array<char, GAMETEXT_BUFFER_8_SIZE>;
    using Utf16Array = rts::array<unichar_t, GAMETEXT_BUFFER_16_SIZE>;
    using Utf8View = rts::array_view<char>;
    using Utf16View = rts::array_view<unichar_t>;

private:
    bool Load(const char *filename, Type filetype, const Languages *languages);
    bool Save(const char *filename, Type filetype, const Languages *languages);

    void Merge_And_Overwrite_Internal(const GameTextFile &other, LanguageID language);
    void Check_Buffer_Lengths(Languages languages);

    StringInfos &Mutable_String_Infos();
    StringInfos &Mutable_String_Infos(LanguageID language);

    template<typename Functor> static void For_Each_Language(Languages languages, Functor functor);
    static StringInfosPtrArray Build_String_Infos_Ptrs_Array(Languages languages, StringInfosArray &string_infos_array);
    static ConstStringInfosPtrArray Build_Const_String_Infos_Ptrs_Array(
        Languages languages, StringInfosArray &string_infos_array);

    static size_t Get_Max_Size(const ConstStringInfosPtrArray &string_infos_ptrs);
    static void Build_Multi_String_Infos(
        MultiStringInfos &multi_string_infos, const ConstStringInfosPtrArray &string_infos_ptrs, Options options);
    static void Build_String_Infos(
        StringInfosPtrArray &string_infos_ptrs, const MultiStringInfos &multi_string_infos, Options options);

    static Type Get_File_Type(const char *filename, Type filetype);

    static void Collect_Length_Info(LengthInfo &len_info, const StringInfos &strings);
    static void Log_Length_Info(const LengthInfo &len_info);
    static void Check_Length_Info(const LengthInfo &len_info);

    static Utf16String &Get_Text(StringInfo &string_info, LanguageID language);
    static Utf16String &Get_Text(MultiStringInfo &string_info, LanguageID language);
    static Utf8String &Get_Speech(StringInfo &string_info, LanguageID language);
    static Utf8String &Get_Speech(MultiStringInfo &string_info, LanguageID language);

    static bool Read_STR_Multi_File(
        FileRef &file, StringInfosPtrArray &string_infos_ptrs, Languages languages, Options options);
    static bool Read_STR_File(FileRef &file, StringInfos &string_infos, Options options);
    template<typename StringInfosType>
    static void Read_STR_File_T(FileRef &file, StringInfosType &string_infos, Options options);
    static StrParseResult Parse_STR_Label(Utf8Array &read, Utf8String &label);
    static StrParseResult Parse_STR_Search(Utf8Array &read);
    static void Parse_STR_Text(Utf8Array &read, Utf16String &text, Options options);
    static void Parse_STR_Speech(Utf8View &read, Utf8String &speech);
    static bool Parse_STR_Language(const char *cstring, LanguageID &language, size_t &parsed_count);
    static bool Is_STR_Pre_Text(Utf8View read);
    static bool Is_STR_Comment(const char *cstring);
    static bool Is_STR_End(const char *cstring);
    static void Change_Step(StrReadStep &step, StrReadStep new_step, const char *&eol_chars);

    static bool Read_CSF_File(FileRef &file, StringInfos &string_infos, LanguageID &language);
    static bool Read_CSF_Header(FileRef &file, StringInfos &string_infos, LanguageID &language);
    static bool Read_CSF_Entry(FileRef &file, StringInfo &string_info);
    static bool Read_CSF_Label(FileRef &file, StringInfo &string_info, int32_t &texts);
    static bool Read_CSF_Text(FileRef &file, StringInfo &string_info);

    static bool Write_STR_Multi_File(
        FileRef &file, const ConstStringInfosPtrArray &string_infos_ptrs, Languages languages, Options options);
    static bool Write_STR_Multi_Entry(FileRef &file,
        const MultiStringInfo &string_info,
        Languages languages,
        Options options,
        Utf8Array &w1,
        Utf8String &w2);
    static bool Write_STR_Language(FileRef &file, LanguageID language);

    static bool Write_STR_File(FileRef &file, const StringInfos &string_infos, Options options);
    static bool Write_STR_Entry(
        FileRef &file, const StringInfo &string_info, Options options, Utf8Array &w1, Utf8String &w2);
    static bool Write_STR_Label(FileRef &file, const Utf8String &label);
    static bool Write_STR_Text(FileRef &file, const Utf16String &text, Options options, Utf8Array &w1, Utf8String &w2);
    static bool Write_STR_Speech(FileRef &file, const Utf8String &speech);
    static bool Write_STR_End(FileRef &file);

    static bool Write_CSF_File(FileRef &file, const StringInfos &string_infos, const LanguageID &language);
    static bool Write_CSF_Header(FileRef &file, const StringInfos &string_infos, const LanguageID &language);
    static bool Write_CSF_Entry(FileRef &file, const StringInfo &string_info, Utf16Array &write16);
    static bool Write_CSF_Label(FileRef &file, const StringInfo &string_info);
    static bool Write_CSF_Text(FileRef &file, const StringInfo &string_info, Utf16Array &write16);

private:
    Options m_options;
    LanguageID m_language;
    StringInfosArray m_stringInfosArray;
};

} // namespace Thyme
