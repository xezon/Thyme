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
#include "gametextfile.h"
#include "subsysteminterface.h"

#define USE_LEGACY_GAMETEXT 1

struct NoString
{
    NoString *next;
    Utf16String text;
};

struct StringLookUp
{
    const Utf8String *label;
    const StringInfo *info;
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

// GameTextManager is self contained and will automatically load and read generals.csf,
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

#if USE_LEGACY_GAMETEXT

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
    int m_maxLabelLen;
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
    StringInfo *m_mapStringInfo;
    StringLookUp *m_mapStringLUT;
    int m_mapTextCount;
    std::vector<Utf8String> m_stringVector;

#else // !USE_LEGACY_GAMETEXT

private:
    bool m_initialized = false;
    bool m_useStringFile = true;
    Utf16String m_failed = U_CHAR("***FATAL*** String Manager failed to initialize properly");

    // Main localization
    GameTextFile m_textFile;
    int m_textCount = 0;
    const StringInfo *m_stringInfo = nullptr;
    StringLookUp *m_stringLUT = nullptr;

    // Map localization
    GameTextFile m_mapTextFile;
    int m_mapTextCount = 0;
    const StringInfo *m_mapStringInfo = nullptr;
    StringLookUp *m_mapStringLUT = nullptr;

    NoString *m_noStringList = nullptr;
    std::vector<Utf8String> m_stringVector;

#endif // USE_LEGACY_GAMETEXT
};

#ifdef GAME_DLL
extern GameTextInterface *&g_theGameText;
#else
extern GameTextInterface *g_theGameText;
#endif
