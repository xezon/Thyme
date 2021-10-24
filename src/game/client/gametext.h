/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief String file handler.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "asciistring.h"
#include "file.h"
#include "subsysteminterface.h"
#include "unicodestring.h"

// This enum applies to RA2/YR and Generals/ZH, BFME ID's are slightly different.
enum class LanguageID : int32_t
{
    US = 0,
    UK,
    GERMAN,
    FRENCH,
    SPANISH,
    ITALIAN,
    JAPANESE,
    JABBER,
    KOREAN,
    CHINESE,
    UNK1,
    BRAZILIAN,
    POLISH,
    UNKNOWN,
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

struct NoString
{
    NoString *next;
    Utf16String text;
};

struct StringInfo
{
    Utf8String label;
    Utf16String text;
    Utf8String speech;
};

struct StringLookUp
{
    Utf8String *label;
    StringInfo *info;
};

// #TODO Original buffer sizes are exceptionally large. Consider decreasing.
constexpr size_t GAMETEXT_BUFFER_SIZE = 10240;
constexpr size_t GAMETEXT_TRANSLATE_SIZE = 20480;

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
    TYPE_AUTO,
    TYPE_CSF,
    TYPE_STR,

    Count
};

// FEATURE: Game text option flags
enum GameTextOption
{
    GAMETEXTOPTION_NONE = 0,
    GAMETEXTOPTION_WRITEOUT_LF = BIT(0),
};

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
    using StringInfos = std::vector<StringInfo>;

public:
    GameTextFile();
    ~GameTextFile();

    bool Load(const char *filename, GameTextType filetype = GameTextType::TYPE_AUTO);
    bool Save(const char *filename, GameTextType filetype = GameTextType::TYPE_AUTO);
    void Unload();

private:
    // Original game functionality
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

    // Thyme functionality
    static const char *Get_File_Extension(const char *filename);
    static GameTextType Get_File_Type(const char *filename, GameTextType filetype);

    static void Log_Length_Info(const GameTextLengthInfo &len_info);
    static void Check_Length_Info(const GameTextLengthInfo &len_info);

    template<typename T> static bool Write(File *file, const T &value);
    template<> static bool Write<Utf8String>(File *file, const Utf8String &string);
    template<> static bool Write<Utf16String>(File *file, const Utf16String &string);
    static bool Write(File *file, const void *data, size_t len);

    static bool Write_STR_Label(File *file, const StringInfo &string_info, GameTextLengthInfo &len_info);
    static bool Write_STR_Text(
        File *file, const StringInfo &string_info, GameTextLengthInfo &len_info, GameTextOption options);
    static bool Write_STR_Speech(File *file, const StringInfo &string_info, GameTextLengthInfo &len_info);
    static bool Write_STR_End(File *file, const StringInfo &string_info, GameTextLengthInfo &len_info);
    static bool Write_STR_Entry(File *file,
        const StringInfo &string_info,
        GameTextLengthInfo &len_info,
        GameTextOption options = GAMETEXTOPTION_NONE);
    static bool Write_STR_File(File *file, BufferView<StringInfo> string_info_bufview, GameTextLengthInfo &len_info);

    static bool Write_CSF_Header(File *file, BufferView<StringInfo> string_info_bufview, LanguageID language);
    static bool Write_CSF_Label(File *file, const StringInfo &string_info, GameTextLengthInfo &len_info);
    static bool Write_CSF_Text(
        File *file, const StringInfo &string_info, GameTextLengthInfo &len_info, BufferView<unichar_t> translate_bufview);
    static bool Write_CSF_Entry(
        File *file, const StringInfo &string_info, GameTextLengthInfo &len_info, BufferView<unichar_t> translate_bufview);
    static bool Write_CSF_File(
        File *file, BufferView<StringInfo> string_info_bufview, GameTextLengthInfo &len_info, LanguageID language);

private:
    LanguageID m_language;
    StringInfos m_stringInfos;

    static const char s_eol[];
    static const char s_quo[];
    static const char s_end[];
};

class GameTextInterface : public SubsystemInterface
{
public:
    virtual ~GameTextInterface() {}

    virtual Utf16String Fetch(const char *args, bool *success = nullptr) = 0;
    virtual Utf16String Fetch(Utf8String args, bool *success = nullptr) = 0;
    virtual std::vector<Utf8String> *Get_Strings_With_Prefix(Utf8String label) = 0;
    virtual void Init_Map_String_File(Utf8String const &filename) = 0;
    virtual void Deinit() = 0;
};

// GameTextManager is an original game class. It is self contained and will automatically load and read generals.csf,
// generals.str and map.str files.
class GameTextManager : public GameTextInterface
{
public:
    GameTextManager();
    virtual ~GameTextManager();

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override {}

    virtual Utf16String Fetch(const char *args, bool *success = nullptr) override;
    virtual Utf16String Fetch(Utf8String args, bool *success = nullptr) override;
    virtual std::vector<Utf8String> *Get_Strings_With_Prefix(Utf8String label) override;
    virtual void Init_Map_String_File(Utf8String const &filename) override;
    virtual void Deinit() override;

    static int Compare_LUT(void const *a, void const *b);
    static GameTextInterface *Create_Game_Text_Interface();

private:
    void Read_To_End_Of_Quote(File *file, char *in, char *out, char *wave, int buff_len);
    void Translate_Copy(unichar_t *out, char *in);
    void Remove_Leading_And_Trailing(char *buffer);
    void Strip_Spaces(unichar_t *buffer);
    void Reverse_Word(char *start, char *end);
    char Read_Char(File *file);
    bool Read_Line(char *buffer, int length, File *file);
    bool Get_String_Count(const char *filename, int &count);
    bool Get_CSF_Info(const char *filename);
    bool Parse_String_File(const char *filename);
    bool Parse_CSF_File(const char *filename);
    bool Parse_Map_String_File(const char *filename);

private:
    int m_textCount;
    int m_maxLabelLen; // #TODO Remove?
    char m_bufferIn[GAMETEXT_BUFFER_SIZE];
    char m_bufferOut[GAMETEXT_BUFFER_SIZE];
    char m_bufferEx[GAMETEXT_BUFFER_SIZE];
    unichar_t m_translateBuffer[GAMETEXT_TRANSLATE_SIZE];
    StringInfo *m_stringInfo;
    StringLookUp *m_stringLUT;
    bool m_initialized;
    // pad 3 chars
    NoString *m_noStringList;
    bool m_useStringFile;
    // pad 3 chars
    LanguageID m_language;
    Utf16String m_failed;
    StringInfo *m_mapStringInfo; // #TODO Replace all allocated arrays and buffers with std::vector
    StringLookUp *m_mapStringLUT;
    int m_mapTextCount;
    std::vector<Utf8String> m_stringVector;
};

#ifdef GAME_DLL
extern GameTextInterface *&g_theGameText;
#else
extern GameTextInterface *g_theGameText;
#endif