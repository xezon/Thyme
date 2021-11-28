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

#include "common/utility/arrayview.h"
#include "common/utility/enumerator.h"
#include "common/utility/flags.h"
#include "fileref.h"
#include "gametextcommon.h"

namespace Thyme
{

enum class GameTextOption : uint8_t
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
    using Languages = rts::numflags<LanguageID, LanguageID::COUNT>;

    GameTextFile() : m_options(Options::Value::OPTIMIZE_MEMORY_SIZE), m_language(LanguageID::UNKNOWN), m_stringInfos(){};

    // Checks whether or not localization is loaded.
    bool Is_Loaded() const;
    bool Is_Loaded(Languages languages) const;

    // Loads CSF or STR file from disk. Does not unload previous data on failure.
    bool Load(const char *filename);
    bool Load_CSF(const char *filename);
    bool Load_STR(const char *filename, const Languages *languages = nullptr, const size_t *size_hint = nullptr);

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

    enum class GameTextReadStep : uint8_t
    {
        LABEL,
        TEXT_BEGIN,
        TEXT_END,
        END,
    };

    struct LengthInfo
    {
        int max_label_len;
        int max_text_utf8_len;
        int max_text_utf16_len;
        int max_speech_len;
    };

    using ConstStringInfosView = rts::array_view<const StringInfos>; // #TODO make std::array ?
    using ConstStringInfosPtrView = rts::array_view<const StringInfos *>; // #TODO make std::array ?
    using StringInfosPtrView = rts::array_view<StringInfos *>; // #TODO make std::array ?
    using ReadStep = rts::enumerator<GameTextReadStep>;
    using Utf8View = rts::array_view<char>;
    using Utf16View = rts::array_view<unichar_t>;

private:
    bool Load(const char *filename, Type filetype, const Languages *languages, const size_t *size_hint);
    bool Save(const char *filename, Type filetype, const Languages *languages);

    void Merge_And_Overwrite_Internal(const GameTextFile &other, LanguageID language);
    void Check_Buffer_Lengths(Languages languages);

    StringInfos &Mutable_String_Infos();
    StringInfos &Mutable_String_Infos(LanguageID language);

    template<typename Functor> static void For_Each_Language(Languages languages, Functor functor);

    static size_t Get_Max_Size(ConstStringInfosPtrView string_infos_ptrs);
    static void Build_Multi_String_Infos(
        MultiStringInfos &multi_string_infos, ConstStringInfosPtrView string_infos_ptrs, Options options);
    static void Build_String_Infos(
        StringInfosPtrView string_infos_ptrs, const MultiStringInfos &multi_string_infos, Options options);

    static Type Get_File_Type(const char *filename, Type filetype);

    static void Collect_Length_Info(LengthInfo &len_info, const StringInfos &strings);
    static void Log_Length_Info(const LengthInfo &len_info);
    static void Check_Length_Info(const LengthInfo &len_info);

    static bool Read_STR_File(FileRef &file, StringInfos &string_infos, Options options, const size_t *size_hint);
    static bool Try_Parse_STR_Label(Utf8View read8, StringInfo &string_info);
    static void Parse_STR_Text_Begin(Utf8View read8, StringInfo &string_info);
    static void Parse_STR_Text_End(Utf8View read8, StringInfo &string_info, Options options);
    static bool Try_Parse_STR_End(Utf8View read8, StringInfo &string_info);
    static bool Is_STR_Comment(const char *cstring);
    static bool Is_STR_End(const char *cstring);
    static void Next_Step(ReadStep &step, char &eol);

    static bool Read_CSF_File(FileRef &file, StringInfos &string_infos, LanguageID &language);
    static bool Read_CSF_Header(FileRef &file, StringInfos &string_infos, LanguageID &language);
    static bool Read_CSF_Entry(FileRef &file, StringInfo &string_info);
    static bool Read_CSF_Label(FileRef &file, StringInfo &string_info, int32_t &texts);
    static bool Read_CSF_Text(FileRef &file, StringInfo &string_info);

    static bool Write_STR_Multi_File(
        FileRef &file, ConstStringInfosView string_infos_view, Languages languages, Options options);
    static bool Write_STR_Multi_File(
        FileRef &file, ConstStringInfosPtrView string_infos_ptrs, Languages languages, Options options);
    static bool Write_STR_Multi_Entry(FileRef &file,
        const MultiStringInfo &string_info,
        Languages languages,
        Options options,
        Utf8View w1,
        Utf8String &w2);
    static bool Write_STR_Language(FileRef &file, LanguageID language);

    static bool Write_STR_File(FileRef &file, const StringInfos &string_infos, Options options);
    static bool Write_STR_Entry(FileRef &file, const StringInfo &string_info, Options options, Utf8View w1, Utf8String &w2);
    static bool Write_STR_Label(FileRef &file, const Utf8String &label);
    static bool Write_STR_Text(FileRef &file, const Utf16String &text, Options options, Utf8View w1, Utf8String &w2);
    static bool Write_STR_Speech(FileRef &file, const Utf8String &speech);
    static bool Write_STR_End(FileRef &file);

    static bool Write_CSF_File(FileRef &file, const StringInfos &string_infos, const LanguageID &language);
    static bool Write_CSF_Header(FileRef &file, const StringInfos &string_infos, const LanguageID &language);
    static bool Write_CSF_Entry(FileRef &file, const StringInfo &string_info, Utf16View write16);
    static bool Write_CSF_Label(FileRef &file, const StringInfo &string_info);
    static bool Write_CSF_Text(FileRef &file, const StringInfo &string_info, Utf16View write16);

private:
    Options m_options;
    LanguageID m_language;
    StringInfos m_stringInfos[size_t(LanguageID::COUNT)];
};

} // namespace Thyme
