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

#include "gametextcommon.h"
#include "utility/stlutil.h"
#include <cstdlib>

namespace Thyme
{

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

struct ConstMultiStringLookup
{
    const char *label;
    const MultiStringInfo *string_info;
};

struct MutableMultiStringLookup
{
    const char *label;
    MultiStringInfo *string_info;
};

// #FEATURE Template class to help find strings by their label within a given string info container. Prefer using the type
// aliases further down below.
template<typename StringLookup, typename StringInfos> class GameTextLookup
{
    using StringLookups = std::vector<StringLookup>;

public:
    explicit GameTextLookup() {}
    explicit GameTextLookup(StringInfos &stringInfos) { Load(stringInfos); }

    void Load(StringInfos &stringInfos)
    {
        const size_t size = stringInfos.size();
        m_stringLookups.resize(size);

        for (size_t i = 0; i < size; ++i) {
            m_stringLookups[i].label = stringInfos[i].label.Str();
            m_stringLookups[i].string_info = &stringInfos[i];
        }

        qsort(&m_stringLookups[0], size, sizeof(StringLookup), Compare_LUT);
    }

    void Unload() { rts::free_container(m_stringLookups); }

    const StringLookup *Find(const char *label) const
    {
        StringLookup key;
        key.label = label;
        key.string_info = nullptr;

        return static_cast<const StringLookup *>(
            bsearch(&key, &m_stringLookups[0], m_stringLookups.size(), sizeof(StringLookup), Compare_LUT));
    }

private:
    static int Compare_LUT(void const *a, void const *b)
    {
        const char *ac = static_cast<const StringLookup *>(a)->label;
        const char *bc = static_cast<const StringLookup *>(b)->label;

        return strcasecmp(ac, bc);
    }

    StringLookups m_stringLookups;
};

// Aliases for convenience.

using ConstGameTextLookup = GameTextLookup<ConstStringLookup, const StringInfos>;
using MutableGameTextLookup = GameTextLookup<MutableStringLookup, StringInfos>;
using ConstMultiGameTextLookup = GameTextLookup<ConstMultiStringLookup, const MultiStringInfos>;
using MutableMultiGameTextLookup = GameTextLookup<MutableMultiStringLookup, MultiStringInfos>;

} // namespace Thyme
