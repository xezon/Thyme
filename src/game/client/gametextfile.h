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
#include <vector>

namespace Thyme
{

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

using StringInfos = std::vector<StringInfo>;

enum class GameTextType
{
    AUTO,
    CSF,
    STR,
};

enum class GameTextOption : uint8_t
{
    NONE = 0,
    CHECK_BUFFER_LENGTH_ON_LOAD = BIT(0),
    CHECK_BUFFER_LENGTH_ON_SAVE = BIT(1),
    KEEP_SPACES_ON_STR_READ = BIT(2),
    PRINT_LINEBREAKS_ON_STR_WRITE = BIT(3),
};

enum class GameTextReadStep : uint8_t
{
    LABEL,
    TEXT_BEGIN,
    TEXT_END,
    END,
};

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
    using Options = rts::bitflags<GameTextOption>;
    using Type = GameTextType;
    using Utf8View = rts::array_view<char>;
    using Utf16View = rts::array_view<unichar_t>;
    using ReadStep = rts::enumerator<GameTextReadStep>;

    GameTextFile() : m_options(Options::Value::NONE), m_language(LanguageID::LANGUAGE_ID_US), m_stringInfos(){};

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
    void SetOptions(Options options) { m_options = options; }
    Options GetOptions() const { return m_options; }

private:
    static Type Get_File_Type(const char *filename, Type filetype);

    static void Collect_Length_Info(LengthInfo &len_info, const StringInfos &strings);
    static void Log_Length_Info(const LengthInfo &len_info);
    static void Check_Length_Info(const LengthInfo &len_info);

    static bool Read_STR_File(FileRef &file, StringInfos &string_infos, Options options);
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

    static bool Write_STR_File(FileRef &file, const StringInfos &string_infos, Options options);
    static bool Write_STR_Entry(FileRef &file, const StringInfo &string_info, Options options, Utf8View w1, Utf8String &w2);
    static bool Write_STR_Label(FileRef &file, const StringInfo &string_info);
    static bool Write_STR_Text(FileRef &file, const StringInfo &string_info, Options options, Utf8View w1, Utf8String &w2);
    static bool Write_STR_Speech(FileRef &file, const StringInfo &string_info);
    static bool Write_STR_End(FileRef &file);

    static bool Write_CSF_File(FileRef &file, const StringInfos &string_infos, const LanguageID &language);
    static bool Write_CSF_Header(FileRef &file, const StringInfos &string_infos, const LanguageID &language);
    static bool Write_CSF_Entry(FileRef &file, const StringInfo &string_info, Utf16View write16);
    static bool Write_CSF_Label(FileRef &file, const StringInfo &string_info);
    static bool Write_CSF_Text(FileRef &file, const StringInfo &string_info, Utf16View write16);

private:
    Options m_options;
    LanguageID m_language;
    StringInfos m_stringInfos;
};

} // namespace Thyme
