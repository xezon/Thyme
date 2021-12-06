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
namespace
{
constexpr const char *const s_localization_us = "English";
constexpr const char *const s_localization_en = "";
constexpr const char *const s_localization_de = "German";
constexpr const char *const s_localization_fr = "French";
constexpr const char *const s_localization_es = "Spanish";
constexpr const char *const s_localization_it = "Italian";
constexpr const char *const s_localization_ja = "";
constexpr const char *const s_localization_jb = "";
constexpr const char *const s_localization_ko = "Korean";
constexpr const char *const s_localization_zh = "Chinese";
constexpr const char *const s_localization___ = "";
constexpr const char *const s_localization_bp = "Brazilian";
constexpr const char *const s_localization_pl = "Polish";
constexpr const char *const s_localization_ru = "Russian";
constexpr const char *const s_localization_ar = "Arabic";

constexpr const char *const s_localizations[] = {
    s_localization_us,
    s_localization_en,
    s_localization_de,
    s_localization_fr,
    s_localization_es,
    s_localization_it,
    s_localization_ja,
    s_localization_jb,
    s_localization_ko,
    s_localization_zh,
    s_localization___,
    s_localization_bp,
    s_localization_pl,
    s_localization___,
    s_localization_ru,
    s_localization_ar,
};

static_assert(s_localization_us == s_localizations[size_t(LanguageID::US)], "Expected language is not set");
static_assert(s_localization_en == s_localizations[size_t(LanguageID::UK)], "Expected language is not set");
static_assert(s_localization_de == s_localizations[size_t(LanguageID::GERMAN)], "Expected language is not set");
static_assert(s_localization_fr == s_localizations[size_t(LanguageID::FRENCH)], "Expected language is not set");
static_assert(s_localization_es == s_localizations[size_t(LanguageID::SPANISH)], "Expected language is not set");
static_assert(s_localization_it == s_localizations[size_t(LanguageID::ITALIAN)], "Expected language is not set");
static_assert(s_localization_ja == s_localizations[size_t(LanguageID::JAPANESE)], "Expected language is not set");
static_assert(s_localization_jb == s_localizations[size_t(LanguageID::JABBER)], "Expected language is not set");
static_assert(s_localization_ko == s_localizations[size_t(LanguageID::KOREAN)], "Expected language is not set");
static_assert(s_localization_zh == s_localizations[size_t(LanguageID::CHINESE)], "Expected language is not set");
static_assert(s_localization___ == s_localizations[size_t(LanguageID::UNUSED_1)], "Expected language is not set");
static_assert(s_localization_bp == s_localizations[size_t(LanguageID::BRAZILIAN)], "Expected language is not set");
static_assert(s_localization_pl == s_localizations[size_t(LanguageID::POLISH)], "Expected language is not set");
static_assert(s_localization___ == s_localizations[size_t(LanguageID::UNKNOWN)], "Expected language is not set");
static_assert(s_localization_ru == s_localizations[size_t(LanguageID::RUSSIAN)], "Expected language is not set");
static_assert(s_localization_ar == s_localizations[size_t(LanguageID::ARABIC)], "Expected language is not set");

static_assert(ARRAY_SIZE(s_localizations) == LanguageCount, "Expected language is not set");

LanguageID Find_Language(const char *localization)
{
    rts::enumerator<LanguageID> languageIt;

    for (const char *name : s_localizations) {
        if (0 == strcasecmp(localization, name)) {
            return languageIt.value();
        }
        ++languageIt;
    }
    return LanguageID::UNKNOWN;
}

} // namespace

// #OBSOLETE Kept for ABI compatibility.
struct StringLookUp
{
    const Utf8String *label;
    const StringInfo *info;
};

// #OBSOLETE Kept for ABI compatibility.
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
    m_textLookup(),
    m_mapTextFile(),
    m_mapTextLookup(),
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

    const Utf8String language_str = Get_Registry_Language();
    bool loaded = false;

    if (m_useStringFile) {
        // Standard behavior.
        m_textFile.Set_Language(LanguageID::UNKNOWN);

        if (m_textFile.Load_STR("data/Generals.str")) {
            loaded = true;
        } else {
            // Non standard behavior. Try loading STR string from different places.
            const LanguageID language = Find_Language(language_str.Str());
            m_textFile.Set_Language(language);

            if (m_textFile.Load_STR("data/Generals.str")) {
                loaded = true;
            } else {
                Utf8String str_file;
                str_file.Format("data/%s/Generals.str", language_str.Str());
                m_textFile.Set_Language(LanguageID::UNKNOWN);

                if (m_textFile.Load_STR(str_file.Str())) {
                    m_textFile.Swap_String_Infos(LanguageID::UNKNOWN, language);
                    m_textFile.Set_Language(language);
                    loaded = true;
                } else {
                    m_textFile.Set_Language(language);

                    if (m_textFile.Load_STR(str_file.Str())) {
                        loaded = true;
                    }
                }
            }
        }
    }

    if (!loaded) {
        Utf8String csf_file;
        csf_file.Format("data/%s/Generals.csf", language_str.Str());
        if (m_textFile.Load_CSF(csf_file.Str())) {
            loaded = true;
        }
    }

    if (!loaded) {
        Deinit();
        return;
    }

    m_textLookup.Load(m_textFile.Get_String_Infos());

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
    m_mapTextLookup.Unload();
    m_mapTextFile.Unload();
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
    if (!m_textFile.Is_Loaded()) {
        if (success != nullptr) {
            *success = false;
        }
        return m_failed;
    }

    const ConstStringLookup *found = m_textLookup.Find(args);

    if (found != nullptr) {
        if (success != nullptr) {
            *success = true;
        }
        return found->string_info->text;
    }

    found = m_textLookup.Find(args);

    if (found != nullptr) {
        if (success != nullptr) {
            *success = true;
        }
        return found->string_info->text;
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
    const char *other_label = label.Str();

    Collect_Labels_With_Prefix(m_stringVector, label, m_textFile.Get_String_Infos());
    Collect_Labels_With_Prefix(m_stringVector, label, m_mapTextFile.Get_String_Infos());

    return &m_stringVector;
}

void GameTextManager::Collect_Labels_With_Prefix(
    Utf8Strings &found_labels, const Utf8String &search_label, const StringInfos &string_infos)
{
    const char *search_label_str = search_label.Str();

    for (const StringInfo &string_info : string_infos) {
        const char *found_label = string_info.label.Str();

        if (strstr(found_label, search_label_str) == found_label) {
            found_labels.push_back(string_info.label);
        }
    }
}

// Initializes a string file associated with a specific map. Resources
// allocated here are freed by GameTextManager::Reset()
void GameTextManager::Init_Map_String_File(Utf8String const &filename)
{
    if (m_mapTextFile.Load_STR(filename.Str())) {
        m_mapTextLookup.Load(m_mapTextFile.Get_String_Infos());
    }
}

// Destroys the main string file, doesn't affect loaded map strings.
void GameTextManager::Deinit()
{
    m_textLookup.Unload();
    m_textFile.Unload();

    for (NoString *ns = m_noStringList; ns != nullptr;) {
        NoString *tmp = ns;
        ns = tmp->next;
        delete tmp;
    }

    m_noStringList = nullptr;
    m_initialized = false;
}
} // namespace Thyme
