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
#include "fileref.h"
#include "gametextcommon.h"
#include <vector>

// https://www.rfc-editor.org/rfc/rfc3629
// In UTF-8, characters from the U+0000..U+10FFFF range (the UTF-16
// accessible range) are encoded using sequences of 1 to 4 octets.
constexpr size_t GAMETEXT_BUFFER_16_SIZE = 1024;
constexpr size_t GAMETEXT_BUFFER_8_SIZE = GAMETEXT_BUFFER_16_SIZE * 4;

// #FEATURE Describes CSF Label header.
struct CSFLabelHeader
{
    uint32_t id;
    int32_t texts;
    int32_t length;
};

// #FEATURE Describes CSF Text header.
struct CSFTextHeader
{
    uint32_t id;
    int32_t length;
};

// #FEATURE Describes CSF Speech header.
struct CSFSpeechHeader
{
    int32_t length;
};

using StringInfos = std::vector<StringInfo>;

// #FEATURE Game text type.
enum class GameTextType
{
    AUTO,
    CSF,
    STR,
};

// #FEATURE Game text option flags.
enum class GameTextOption
{
    NONE = 0,
    PRINT_LENGTH_INFO_ON_LOAD = BIT(0),
    PRINT_LENGTH_INFO_ON_SAVE = BIT(1),
};

DEFINE_ENUMERATION_BITWISE_OPERATORS(GameTextOption)

// #FEATURE Stores information about localization text lengths.
struct GameTextLengthInfo
{
    int max_label_len;
    int max_text_utf8_len;
    int max_text_utf16_len;
    int max_speech_len;
};

// #FEATURE GameTextFile contains the core file handling functionality of original GameTextManager, which allows to use it
// for more flexible localization file operations.
class GameTextFile
{
public:
    using LengthInfo = GameTextLengthInfo;
    using Option = GameTextOption;
    using Type = GameTextType;
    using Utf8Buf = rts::array_view<char>;
    using Utf16Buf = rts::array_view<unichar_t>;

    GameTextFile() : m_options(Option::NONE), m_language(LanguageID::LANGUAGE_ID_US), m_stringInfos(){};

    // Checks whether or not localization is loaded.
    bool IsLoaded() const { return !m_stringInfos.empty(); }

    // Loads CSF or STR file from disk. Will always unload current file data.
    bool Load(const char *filename, Type filetype = Type::AUTO);

    // Saves CSF or STR file to disk. Will write over any existing file.
    bool Save(const char *filename, Type filetype = Type::AUTO);

    // Unloads current file data.
    void Unload();

    // Merges another game text into this one. Identical labels will write their text contents from the other to this.
    void Merge_And_Overwrite(const GameTextFile &other);

    // Retrieves all localization data.
    const StringInfos &Get_String_Infos() const { return m_stringInfos; }

    // Sets and gets options for operations.
    void SetOptions(Option options) { m_options = options; }
    Option GetOptions() const { return m_options; }

private:
    static void Read_To_End_Of_Quote(File *file, char *in, char *out, char *wave, int buff_len);
    static void Translate_Copy(unichar_t *out, const char *in);
    static void Remove_Leading_And_Trailing(char *buffer);
    static void Strip_Spaces(unichar_t *buffer);
    static char Read_Char(File *file);
    static bool Read_Line(char *buffer, int length, File *file);
    static bool Get_String_Count(const char *filename,
        int &count,
        rts::array_view<char> buffer_in,
        rts::array_view<char> buffer_out,
        rts::array_view<char> buffer_ex);
    static bool Parse_String_File(const char *filename,
        StringInfo *string_info,
        int &max_label_len,
        rts::array_view<unichar_t> buffer_trans,
        rts::array_view<char> buffer_in,
        rts::array_view<char> buffer_out,
        rts::array_view<char> buffer_ex);

    static Type Get_File_Type(const char *filename, Type filetype);

    static void Collect_Length_Info(LengthInfo &len_info, const StringInfos &strings);
    static void Log_Length_Info(const LengthInfo &len_info);
    static void Check_Length_Info(const LengthInfo &len_info);


    static bool Read_CSF_File(FileRef &file, StringInfos &string_infos, LanguageID &language);
    static bool Read_CSF_Header(FileRef &file, StringInfos &string_infos, LanguageID &language);
    static bool Read_CSF_Entry(FileRef &file, StringInfo &string_info);
    static bool Read_CSF_Label(FileRef &file, StringInfo &string_info, int32_t &texts);
    static bool Read_CSF_Text(FileRef &file, StringInfo &string_info);

    static bool Write_STR_File(FileRef &file, const StringInfos &string_infos);
    static bool Write_STR_Entry(FileRef &file, const StringInfo &string_info);
    static bool Write_STR_Label(FileRef &file, const StringInfo &string_info);
    static bool Write_STR_Text(FileRef &file, const StringInfo &string_info);
    static bool Write_STR_Speech(FileRef &file, const StringInfo &string_info);
    static bool Write_STR_End(FileRef &file);

    static bool Write_CSF_File(FileRef &file, const StringInfos &string_infos, const LanguageID &language);
    static bool Write_CSF_Header(FileRef &file, const StringInfos &string_infos, const LanguageID &language);
    static bool Write_CSF_Entry(FileRef &file, const StringInfo &string_info, Utf16Buf utf16buf);
    static bool Write_CSF_Label(FileRef &file, const StringInfo &string_info);
    static bool Write_CSF_Text(FileRef &file, const StringInfo &string_info, Utf16Buf utf16buf);

private:
    Option m_options;
    LanguageID m_language;
    StringInfos m_stringInfos;

    static const char s_eol[];
    static const char s_quo[];
    static const char s_end[];
};
