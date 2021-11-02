/**
 * @file
 *
 * @author xezon
 *
 * @brief Localization String file (Thyme Feature).
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "asciistring.h"
#include "fileref.h"
#include "unicodestring.h"
#include <stdlib.h>
#include <vector>

// https://www.rfc-editor.org/rfc/rfc3629
// In UTF-8, characters from the U+0000..U+10FFFF range (the UTF-16
// accessible range) are encoded using sequences of 1 to 4 octets.
constexpr size_t GAMETEXT_BUFFER_16_SIZE = 1024;
constexpr size_t GAMETEXT_BUFFER_8_SIZE = GAMETEXT_BUFFER_16_SIZE * 4;

// This enum applies to RA2/YR and Generals/ZH, BFME ID's are slightly different.
enum class LanguageID : int32_t
{
    LANGUAGE_ID_US = 0,
    LANGUAGE_ID_UK,
    LANGUAGE_ID_GERMAN,
    LANGUAGE_ID_FRENCH,
    LANGUAGE_ID_SPANISH,
    LANGUAGE_ID_ITALIAN,
    LANGUAGE_ID_JAPANESE,
    LANGUAGE_ID_JABBER,
    LANGUAGE_ID_KOREAN,
    LANGUAGE_ID_CHINESE,
    LANGUAGE_ID_UNK1,
    LANGUAGE_ID_BRAZILIAN,
    LANGUAGE_ID_POLISH,
    LANGUAGE_ID_UNKNOWN,
};

struct CSFHeader
{
    uint32_t id;
    int32_t version;
    int32_t num_labels;
    int32_t num_strings;
    int32_t skip;
    LanguageID langid;
};

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

// #TODO Support reading and writing multiple strings.

struct StringInfo
{
    Utf8String label;
    Utf16String text;
    Utf8String speech;
};

using StringInfos = std::vector<StringInfo>;

struct ConstStringLookup
{
    const char *label;
    const StringInfo *string_info;
};

struct MutableStringLookup
{
    const char *label;
    StringInfo *string_info;
};

// #FEATURE BufferView allows to pass along a buffer and its size in one go.
template<typename ValueType, typename SizeType = int> class BufferView
{
public:
    using value_type = ValueType;
    using size_type = SizeType;

    template<size_t Size> static BufferView Create(value_type (&buf)[Size]) { return BufferView(buf, Size); }
    static BufferView Create(std::vector<value_type> &vector) { return BufferView(&vector[0], vector.size()); }
    static BufferView Create(value_type *buf, size_type size) { return BufferView(buf, size); }

    inline BufferView(value_type *buf, size_type size) : m_buf(buf), m_size(size) {}

    inline operator value_type *() { return m_buf; }
    inline value_type *Get() { return m_buf; }
    inline size_type Size() { return m_size; }

private:
    value_type *m_buf;
    size_type m_size;
};

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
    WRITEOUT_LF = BIT(2),
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
    friend class GameTextManager;

public:
    using LengthInfo = GameTextLengthInfo;
    using Option = GameTextOption;
    using Type = GameTextType;
    using Utf16Buf = BufferView<unichar_t>;

    GameTextFile() : m_options(Option::NONE), m_language(LanguageID::LANGUAGE_ID_US), m_stringInfos(){};

    // Loads CSF or STR file from disk. Will always unload current file data.
    bool Load(const char *filename, Type filetype = Type::AUTO);

    // Saves CSF or STR file to disk. Will write over any existing file.
    bool Save(const char *filename, Type filetype = Type::AUTO);

    // Unloads current file data.
    void Unload();

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
        BufferView<char> buffer_in,
        BufferView<char> buffer_out,
        BufferView<char> buffer_ex);
    static bool Get_CSF_Info(const char *filename, int &text_count, LanguageID &language);
    static bool Parse_String_File(const char *filename,
        StringInfo *string_info,
        int &max_label_len,
        BufferView<unichar_t> buffer_trans,
        BufferView<char> buffer_in,
        BufferView<char> buffer_out,
        BufferView<char> buffer_ex);
    static bool Parse_CSF_File(const char *filename,
        StringInfo *string_info,
        int &max_label_len,
        BufferView<unichar_t> buffer_trans,
        BufferView<char> buffer_in);

    static const char *Get_File_Extension(const char *filename);
    static Type Get_File_Type(const char *filename, Type filetype);

    static void Collect_Length_Info(LengthInfo &len_info, const StringInfos &strings);
    static void Log_Length_Info(const LengthInfo &len_info);
    static void Check_Length_Info(const LengthInfo &len_info);

    template<typename T> static bool Read(FileRef &file, T &value);
    static bool Read(FileRef &file, Utf8String &string, int len);
    static bool Read(FileRef &file, Utf16String &string, int len);
    static bool Read(FileRef &file, void *data, int len);

    template<typename T> static bool Write(FileRef &file, const T &value);
    template<> static bool Write<Utf8String>(FileRef &file, const Utf8String &string);
    template<> static bool Write<Utf16String>(FileRef &file, const Utf16String &string);
    static bool Write(FileRef &file, const void *data, int len);

    bool Read_CSF_File(FileRef &file);
    static bool Read_CSF_Header(FileRef &file, LanguageID &language, StringInfos &strings);
    static bool Read_CSF_Entry(FileRef &file, StringInfo &string_info);
    static bool Read_CSF_Label(FileRef &file, StringInfo &string_info, int32_t &texts);
    static bool Read_CSF_Text(FileRef &file, StringInfo &string_info);

    bool Write_STR_File(FileRef &file);
    static bool Write_STR_Entry(FileRef &file, const StringInfo &string_info, Option options);
    static bool Write_STR_Label(FileRef &file, const StringInfo &string_info);
    static bool Write_STR_Text(FileRef &file, const StringInfo &string_info, Option options);
    static bool Write_STR_Speech(FileRef &file, const StringInfo &string_info);
    static bool Write_STR_End(FileRef &file, const StringInfo &string_info);

    bool Write_CSF_File(FileRef &file);
    static bool Write_CSF_Header(FileRef &file, const LanguageID &language, const StringInfos &strings);
    static bool Write_CSF_Entry(FileRef &file, const StringInfo &string_info, Utf16Buf utf16bufview);
    static bool Write_CSF_Label(FileRef &file, const StringInfo &string_info);
    static bool Write_CSF_Text(FileRef &file, const StringInfo &string_info, Utf16Buf utf16bufview);

private:
    Option m_options;
    LanguageID m_language;
    StringInfos m_stringInfos;

    static const char s_eol[];
    static const char s_quo[];
    static const char s_end[];
};

template<typename StringLookup, typename StringInfos> class GameTextLookup
{
public:
    GameTextLookup(StringInfos &stringInfos);

    const StringLookup *Find(const char *label) const;

private:
    static int Compare_LUT(void const *a, void const *b);

    std::vector<StringLookup> m_stringLUT;
};

template<typename StringLookup, typename StringInfos>
GameTextLookup<StringLookup, StringInfos>::GameTextLookup(StringInfos &stringInfos)
{
    const size_t size = stringInfos.size();
    m_stringLUT.resize(size);

    for (size_t i = 0; i < size; ++i) {
        m_stringLUT[i].label = stringInfos[i].label.Str();
        m_stringLUT[i].string_info = &stringInfos[i];
    }

    qsort(&m_stringLUT[0], size, sizeof(StringLookup), Compare_LUT);
}

template<typename StringLookup, typename StringInfos>
const StringLookup *GameTextLookup<StringLookup, StringInfos>::Find(const char *label) const
{
    StringLookup key;
    key.label = label;
    key.string_info = nullptr;

    return static_cast<const StringLookup *>(
        bsearch(&key, &m_stringLUT[0], m_stringLUT.size(), sizeof(StringLookup), Compare_LUT));
}

template<typename StringLookup, typename StringInfos>
int GameTextLookup<StringLookup, StringInfos>::Compare_LUT(const void *a, const void *b)
{
    const char *ac = static_cast<const StringLookup *>(a)->label;
    const char *bc = static_cast<const StringLookup *>(b)->label;

    return strcasecmp(ac, bc);
}

using ConstGameTextLookup = GameTextLookup<ConstStringLookup, const StringInfos>;
using MutableGameTextLookup = GameTextLookup<MutableStringLookup, StringInfos>;
