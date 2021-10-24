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
#include "file.h"
#include "unicodestring.h"
#include <vector>

// #TODO Original buffer sizes are exceptionally large. Consider decreasing.
constexpr size_t GAMETEXT_BUFFER_SIZE = 10240;
constexpr size_t GAMETEXT_TRANSLATE_SIZE = 20480;

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

struct CSFLabelHeader
{
    uint32_t id;
    int32_t string_count;
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

struct StringInfo
{
    Utf8String label;
    Utf16String text;
    Utf8String speech;
};

// FEATURE: BufferView allows to pass along a buffer and its size in one go.
template<typename T> class BufferView
{
public:
    using value_type = T;

    template<size_t Size> static BufferView Create(value_type (&buf)[Size]) { return BufferView(buf, Size); }
    static BufferView Create(std::vector<T> &vector) { return BufferView(&vector[0], vector.size()); }
    static BufferView Create(value_type *buf, size_t size) { return BufferView(buf, size); }

    inline BufferView(value_type *buf, size_t size) : m_buf(buf), m_size(size) {}

    inline operator value_type *() { return m_buf; }
    inline value_type *Get() { return m_buf; }
    inline size_t Size() { return m_size; }

private:
    value_type *m_buf;
    size_t m_size;
};

// FEATURE: Game text type
enum class GameTextType
{
    AUTO,
    CSF,
    STR,

    Count
};

// FEATURE: Game text option flags
enum class GameTextOption
{
    NONE = 0,
    WRITEOUT_LF = BIT(0),
};

DEFINE_ENUMERATION_BITWISE_OPERATORS(GameTextOption)

struct GameTextLengthInfo
{
    int max_label_len;
    int max_text_utf8_len;
    int max_text_utf16_len;
    int max_speech_len;
};

// FEATURE: GameTextFile contains the core file handling functionality of original GameTextManager, which allows to use it
// for more flexible localization file operations.
class GameTextFile
{
    friend class GameTextManager;

public:
    using StringInfos = std::vector<StringInfo>;
    using LengthInfo = GameTextLengthInfo;
    using Option = GameTextOption;
    using Type = GameTextType;
    using Utf16Buf = BufferView<unichar_t>;

    GameTextFile() : m_options(Option::NONE), m_language(LanguageID::LANGUAGE_ID_US), m_stringInfos(){};

    bool Load(const char *filename, Type filetype = Type::AUTO);
    bool Save(const char *filename, Type filetype = Type::AUTO);
    void Unload();

    const StringInfos &Get_String_Infos() const { return m_stringInfos; }
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

    static void Log_Length_Info(const LengthInfo &len_info);
    static void Check_Length_Info(const LengthInfo &len_info);

    template<typename T> static bool Write(File *file, const T &value);
    template<> static bool Write<Utf8String>(File *file, const Utf8String &string);
    template<> static bool Write<Utf16String>(File *file, const Utf16String &string);
    static bool Write(File *file, const void *data, size_t len);

    bool Write_STR_Label(File *file, const StringInfo &string_info, LengthInfo &len_info);
    bool Write_STR_Text(File *file, const StringInfo &string_info, LengthInfo &len_info);
    bool Write_STR_Speech(File *file, const StringInfo &string_info, LengthInfo &len_info);
    bool Write_STR_End(File *file, const StringInfo &string_info, LengthInfo &len_info);
    bool Write_STR_Entry(File *file, const StringInfo &string_info, LengthInfo &len_info);
    bool Write_STR_File(File *file, LengthInfo &len_info);

    bool Write_CSF_Header(File *file);
    bool Write_CSF_Label(File *file, const StringInfo &string_info, LengthInfo &len_info);
    bool Write_CSF_Text(File *file, const StringInfo &string_info, LengthInfo &len_info, Utf16Buf translate_bufview);
    bool Write_CSF_Entry(File *file, const StringInfo &string_info, LengthInfo &len_info, Utf16Buf translate_bufview);
    bool Write_CSF_File(File *file, LengthInfo &len_info);

private:
    Option m_options;
    LanguageID m_language;
    StringInfos m_stringInfos;

    static const char s_eol[];
    static const char s_quo[];
    static const char s_end[];
};
