/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Weather control device.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "weather.h"
#include <captainslog.h>

#ifndef GAME_DLL
Override<WeatherSetting> g_theWeatherSetting;
#endif

// clang-format off
const FieldParse WeatherSetting::s_weatherSettingParseTable[] = {
    FIELD_PARSE_ASCIISTRING("SnowTexture", WeatherSetting, m_snowTexture),
    { "SnowFrequencyScaleX", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowFreqScaleX) },
    { "SnowFrequencyScaleY", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowFreqScaleY) },
    { "SnowAmplitude", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowAmplitude) },
    { "SnowPointSize", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowPointSize) },
    { "SnowMaxPointSize", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowMaxPointSize) },
    { "SnowMinPointSize", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowMinPointSize) },
    { "SnowQuadSize", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowQuadSize) },
    { "SnowBoxDimensions", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowBoxDimensions) },
    { "SnowBoxDensity", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowBoxDensity) },
    { "SnowVelocity", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowVelocity) },
    { "SnowPointSprites", &INI::Parse_Bool, nullptr, offsetof(WeatherSetting, m_snowPointSprites) },
    { "SnowEnabled", &INI::Parse_Bool, nullptr, offsetof(WeatherSetting, m_snowEnabled) },
    FIELD_PARSE_LAST
};
// clang-format on

WeatherSetting::WeatherSetting() :
    m_snowTexture("EXSnowFlake.tga"),
    m_snowFreqScaleX(0.0533f),
    m_snowFreqScaleY(0.0275f),
    m_snowAmplitude(5.0f),
    m_snowPointSize(1.0f),
    m_snowMaxPointSize(64.0f),
    m_snowMinPointSize(0.0f),
    m_snowQuadSize(0.5f),
    m_snowBoxDimensions(200.0f),
    m_snowBoxDensity(1.0f),
    m_snowVelocity(4.0f),
    m_snowPointSprites(true),
    m_snowEnabled(false)
{
}

// Was originally INI::parseWeatherDefinition
void WeatherSetting::Parse_Weather_Definition(INI *ini)
{
    if (*g_theWeatherSetting) {
        if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
            WeatherSetting *old_ws = g_theWeatherSetting;
            WeatherSetting *new_ws = NEW_POOL_OBJ(WeatherSetting);
            *new_ws = *old_ws;
            new_ws->Set_Is_Allocated();
            old_ws->Friend_Get_Final_Override()->Set_Next(new_ws);
        } else {
            throw CODE_06;
        }
    } else {
        g_theWeatherSetting = NEW_POOL_OBJ(WeatherSetting);
    }

    Overridable *setting = g_theWeatherSetting;
    setting = setting->Friend_Get_Final_Override();
    ini->Init_From_INI(setting, s_weatherSettingParseTable);
    // TODO requires SnowManager virtual table layout implementing.
    // if (g_theSnowManager)
    // {
    //     g_theSnowManager->Update_INI_Settings();
    // }
}
