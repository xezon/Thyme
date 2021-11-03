/**
 * @file
 *
 * @author OmniBlade
 * @author xezon
 *
 * @brief Game Localization Manager with Thyme specific functionality.
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
#include "gametextcommon.h"
#include "gametextfile.h"
#include "gametextinterface.h"

namespace Thyme
{
struct StringLookUp // TODO remove
{
    const Utf8String *label;
    const StringInfo *info;
};

// #FEATURE New GameTextManager with new features. Can read UTF-8 STR Files. GameTextManager is self contained and will
// automatically load and read generals.csf, generals.str and map.str files.
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
};
} // namespace Thyme

using GameTextManager = Thyme::GameTextManager;
