/**
 * @file
 *
 * @author xezon
 *
 * @brief Game Localization Lookup (Thyme Feature).
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
#include <stdlib.h>
#include <vector>

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

// #FEATURE Template class to help find strings by their label within a given string info container.
// Prefer use the type aliases further down below.
template<typename StringLookup, typename StringInfos> class GameTextLookup
{
public:
    GameTextLookup(StringInfos &stringInfos);

    const StringLookup *Find(const char *label) const;

private:
    static int Compare_LUT(void const *a, void const *b);

    std::vector<StringLookup> m_stringLUT;
};

// Aliases.
using ConstGameTextLookup = GameTextLookup<ConstStringLookup, const StringInfos>;
using MutableGameTextLookup = GameTextLookup<MutableStringLookup, StringInfos>;

// Implementation...
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
