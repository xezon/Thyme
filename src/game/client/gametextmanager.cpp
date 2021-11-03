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
#include "gametextmanager.h"
#include "filesystem.h"
#include "main.h" // For g_applicationHWnd
#include "registry.h"
#include "rtsutils.h"
#include <algorithm>
#include <captainslog.h>

#ifdef PLATFORM_WINDOWS
#include <winuser.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef GAME_DLL
#include "hookcrt.h"
#endif

namespace Thyme
{
// Comparison function used for sorting and searching StringLookUp arrays.
int GameTextManager::Compare_LUT(void const *a, void const *b)
{
    const char *ac = static_cast<StringLookUp const *>(a)->label->Str();
    const char *bc = static_cast<StringLookUp const *>(b)->label->Str();

    return strcasecmp(ac, bc);
}

GameTextInterface *GameTextManager::Create_Game_Text_Interface()
{
    return new GameTextManager;
}

GameTextManager::GameTextManager() :
    m_initialized(false),
    m_useStringFile(true),
    m_failed(U_CHAR("***FATAL*** String Manager failed to initialize properly")),
    m_textFile(),
    m_textCount(0),
    m_stringInfo(nullptr),
    m_stringLUT(nullptr),
    m_mapTextFile(),
    m_mapTextCount(0),
    m_mapStringInfo(nullptr),
    m_mapStringLUT(nullptr),
    m_noStringList(nullptr),
    m_stringVector()
{
}

GameTextManager::~GameTextManager()
{
    Deinit();
    Reset();
}

void GameTextManager::Init()
{
    captainslog_info("Initializing GameTextManager.");
    if (m_initialized) {
        return;
    }

    bool loaded = false;

    if (m_useStringFile && m_textFile.Load("data/Generals.str", GameTextType::STR)) {
        loaded = true;
    } else {
        Utf8String csf_file;
        csf_file.Format("data/%s/Generals.csf", Get_Registry_Language().Str());
        if (m_textFile.Load(csf_file.Str(), GameTextType::CSF)) {
            loaded = true;
        }
    }

    if (!loaded) {
        Deinit();
        return;
    }

    m_textCount = m_textFile.Get_String_Infos().size();
    m_stringInfo = &m_textFile.Get_String_Infos()[0];

    // Generate the lookup table and sort it for efficient search.
    m_stringLUT = new StringLookUp[m_textCount];

    for (int i = 0; i < m_textCount; ++i) {
        m_stringLUT[i].info = &m_stringInfo[i];
        m_stringLUT[i].label = &m_stringInfo[i].label;
    }

    qsort(m_stringLUT, m_textCount, sizeof(StringLookUp), Compare_LUT);

    // Fetch the GUI window title string and set it here.
    Utf8String ntitle;
    Utf16String wtitle;
    // #FEATURE Add Thyme text to window name.
    wtitle = U_CHAR("Thyme - ");
    wtitle += Fetch("GUI:Command&ConquerGenerals");

    ntitle.Translate(wtitle);

#ifdef PLATFORM_WINDOWS
    if (g_applicationHWnd != 0) {
        SetWindowTextA(g_applicationHWnd, ntitle.Str());
        SetWindowTextW(g_applicationHWnd, (const wchar_t *)wtitle.Str());
    }
#endif
}

// Resets the map strings, main string file is left loaded.
void GameTextManager::Reset()
{
    // BUGFIX: Reset map text count as well.
    m_mapTextCount = 0;

#if USE_LEGACY_GAMETEXT
    if (m_mapStringInfo != nullptr) {
        delete[] m_mapStringInfo;
        m_mapStringInfo = nullptr;
    }
#else
    m_mapStringInfo = nullptr;
    m_mapTextFile.Unload();
#endif

    if (m_mapStringLUT != nullptr) {
        delete[] m_mapStringLUT;
        m_mapStringLUT = nullptr;
    }
}

// Find and return the unicode string corresponding to the label provided.
// Optionally can pass a bool pointer to determine if a string was found.
Utf16String GameTextManager::Fetch(Utf8String args, bool *success)
{
    return Fetch(args.Str(), success);
}

// Find and return the unicode string corresponding to the label provided.
// Optionally can pass a bool pointer to determine if a string was found.
Utf16String GameTextManager::Fetch(const char *args, bool *success)
{
    if (m_stringInfo == nullptr) {
        if (success != nullptr) {
            *success = false;
        }

        return m_failed;
    }

    Utf8String argstr = args;
    StringLookUp key = { &argstr, nullptr };

    StringLookUp *found =
        static_cast<StringLookUp *>(bsearch(&key, m_stringLUT, m_textCount, sizeof(StringLookUp), Compare_LUT));

    if (found != nullptr) {
        if (success != nullptr) {
            *success = true;
        }

        return found->info->text;
    }

    if (m_mapStringLUT != nullptr && m_mapTextCount > 0) {
        found =
            static_cast<StringLookUp *>(bsearch(&key, m_mapStringLUT, m_mapTextCount, sizeof(StringLookUp), Compare_LUT));

        if (found != nullptr) {
            if (success != nullptr) {
                *success = true;
            }

            return found->info->text;
        }
    }

    if (success != nullptr) {
        *success = false;
    }

    // If we reached here, we didn't find a string from our string file.
    Utf16String missing;
    NoString *no_string;

    missing.Format(U_CHAR("MISSING: '%hs'"), args);

    // Find missing string in NoString list if it already exists.
    for (no_string = m_noStringList; no_string != nullptr; no_string = no_string->next) {
        if (missing == no_string->text) {
            break;
        }
    }

    // If it was not found or the list was empty, add a new one.
    if (no_string == nullptr) {
        no_string = new NoString;
        no_string->text = missing;
        no_string->next = m_noStringList;
        m_noStringList = no_string;
    }

    return no_string->text;
}

// List all string labels that start with the prefix passed to the function.
std::vector<Utf8String> *GameTextManager::Get_Strings_With_Prefix(Utf8String label)
{
    m_stringVector.clear();

    captainslog_trace("Searching for strings prefixed with '%s'.", label.Str());

    // Search all string labels that start with the substring provided.
    if (m_stringLUT != nullptr) {
        for (int i = 0; i < m_textCount; ++i) {
            const char *lut_label = m_stringLUT[i].label->Str();

            if (strstr(lut_label, label.Str()) == lut_label) {
                m_stringVector.push_back(*m_stringLUT[i].label);
            }
        }
    }

    // Same again for map strings.
    if (m_mapStringLUT != nullptr) {
        for (int i = 0; i < m_mapTextCount; ++i) {
            const char *lut_label = m_mapStringLUT[i].label->Str();
            if (strstr(lut_label, label.Str()) == lut_label) {
                m_stringVector.push_back(*m_mapStringLUT[i].label);
            }
        }
    }

    return &m_stringVector;
}

// Initializes a string file associated with a specific map. Resources
// allocated here are freed by GameTextManager::Reset()
void GameTextManager::Init_Map_String_File(Utf8String const &filename)
{
    m_mapTextFile.Load(filename.Str());
    m_mapTextCount = m_mapTextFile.Get_String_Infos().size();
    m_mapStringInfo = &m_mapTextFile.Get_String_Infos()[0];

    // Generate the lookup table and sort it for efficient search.
    m_mapStringLUT = new StringLookUp[m_mapTextCount];

    for (int i = 0; i < m_mapTextCount; ++i) {
        m_mapStringLUT[i].info = &m_mapStringInfo[i];
        m_mapStringLUT[i].label = &m_mapStringInfo[i].label;
    }

    qsort(m_mapStringLUT, m_mapTextCount, sizeof(StringLookUp), Compare_LUT);
}

// Destroys the main string file, doesn't affect loaded map strings.
void GameTextManager::Deinit()
{
    m_stringInfo = nullptr;
    m_textFile.Unload();

    if (m_stringLUT != nullptr) {
        delete[] m_stringLUT;
        m_stringLUT = nullptr;
    }

    m_textCount = 0;

    for (NoString *ns = m_noStringList; ns != nullptr;) {
        NoString *tmp = ns;
        ns = tmp->next;

        delete tmp;
    }

    m_noStringList = nullptr;
    m_initialized = false;
}
} // namespace Thyme
