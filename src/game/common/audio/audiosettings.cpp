/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class holding the settings for the audio engine.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "audiosettings.h"
#include "audiomanager.h"
#include "optionpreferences.h"
#include <cstddef>

// clang-format off
const FieldParse AudioSettings::s_audioSettingsParseTable[] = {
    FIELD_PARSE_ASCIISTRING("AudioRoot", AudioSettings, m_audioRoot),
    FIELD_PARSE_ASCIISTRING("SoundsFolder", AudioSettings, m_soundsFolder),
    FIELD_PARSE_ASCIISTRING("MusicFolder", AudioSettings, m_musicFolder),
    FIELD_PARSE_ASCIISTRING("StreamingFolder", AudioSettings, m_streamingFolder),
    FIELD_PARSE_ASCIISTRING("SoundsExtension", AudioSettings, m_soundExtension),
    { "UseDigital", &INI::Parse_Bool, nullptr, offsetof(AudioSettings, m_useDigital) },
    { "UseMidi", &INI::Parse_Bool, nullptr, offsetof(AudioSettings, m_useMidi) },
    { "OutputRate", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_outputRate) },
    { "OutputBits", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_outputBits) },
    { "OutputChannels", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_outputChannels) },
    { "SampleCount2D", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_sampleCount2D) },
    { "SampleCount3D", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_sampleCount3D) },
    { "StreamCount", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_streamCount) },
    FIELD_PARSE_ASCIISTRING("Preferred3DHW1", AudioSettings, m_preferredDrivers[0]),
    FIELD_PARSE_ASCIISTRING("Preferred3DHW2", AudioSettings, m_preferredDrivers[1]),
    FIELD_PARSE_ASCIISTRING("Preferred3DHW3", AudioSettings, m_preferredDrivers[2]),
    FIELD_PARSE_ASCIISTRING("Preferred3DHW4", AudioSettings, m_preferredDrivers[3]),
    FIELD_PARSE_ASCIISTRING("Preferred3DSW", AudioSettings, m_preferredDrivers[4]),
    { "Default2DSpeakerType", &INI::Parse_Speaker_Type, nullptr, offsetof(AudioSettings, m_default2DSpeakerType) },
    { "Default3DSpeakerType", &INI::Parse_Speaker_Type, nullptr, offsetof(AudioSettings, m_default3DSpeakerType) },
    { "MinSampleVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_minSampleVolume) },
    { "GlobalMinRange", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_globalMinRange) },
    { "GlobalMaxRange", &INI::Parse_Int, nullptr, offsetof(AudioSettings, m_globalMaxRange) },
    { "TimeBetweenDrawableSounds",
        &INI::Parse_Duration_Unsigned_Int,
        nullptr,
        offsetof(AudioSettings, m_timeBetweenDrawableSounds) },
    { "TimeToFadeAudio", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(AudioSettings, m_timeToFadeAudio) },
    { "AudioFootprintInBytes", &INI::Parse_Unsigned_Int, nullptr, offsetof(AudioSettings, m_audioFootprintInBytes) },
    { "Relative2DVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_relative2DVolume) },
    { "DefaultSoundVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_defaultSoundVolume) },
    { "Default3DSoundVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_default3DSoundVolume) },
    { "DefaultSpeechVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_defaultSpeechVolume) },
    { "DefaultMusicVolume", &INI::Parse_Percent_To_Real, nullptr, offsetof(AudioSettings, m_defaultMusicVolume) },
    { "MicrophoneDesiredHeightAboveTerrain",
        &INI::Parse_Real,
        nullptr,
        offsetof(AudioSettings, m_microphoneDesiredHeightAboveTerrain) },
    { "MicrophoneMaxPercentageBetweenGroundAndCamera",
        &INI::Parse_Percent_To_Real,
        nullptr,
        offsetof(AudioSettings, m_microphoneMaxPercentBetweenGroundAndCamera) },
    { "ZoomMinDistance", &INI::Parse_Real, nullptr, offsetof(AudioSettings, m_zoomMinDistance) },
    { "ZoomMaxDistance", &INI::Parse_Real, nullptr, offsetof(AudioSettings, m_zoomMaxDistance) },
    { "ZoomSoundVolumePercentageAmount",
        &INI::Parse_Percent_To_Real,
        nullptr,
        offsetof(AudioSettings, m_zoomSoundVolumePercentAmount) },
    FIELD_PARSE_LAST
};
// clang-format on

// wb: 0x006E5C20
AudioSettings::AudioSettings() :
    // #BUGFIX Initialize all members
    m_audioRoot(),
    m_soundsFolder(),
    m_musicFolder(),
    m_streamingFolder(),
    m_soundExtension(),
    m_useDigital{},
    m_useMidi{},
    m_outputRate{},
    m_outputBits{},
    m_outputChannels{},
    m_sampleCount2D{},
    m_sampleCount3D{},
    m_streamCount{},
    m_globalMinRange{},
    m_globalMaxRange{},
    m_timeBetweenDrawableSounds{},
    m_timeToFadeAudio{},
    m_audioFootprintInBytes{},
    m_minSampleVolume{},
    m_preferredDrivers(),
    m_relative2DVolume{},
    m_defaultSoundVolume{},
    m_default3DSoundVolume{},
    m_defaultSpeechVolume{},
    m_defaultMusicVolume{},
    m_default2DSpeakerType{},
    m_default3DSpeakerType{},
    m_soundVolume{},
    m_3dSoundVolume{},
    m_speechVolume{},
    m_musicVolume{},
    m_microphoneDesiredHeightAboveTerrain{},
    m_microphoneMaxPercentBetweenGroundAndCamera{},
    m_zoomMinDistance{},
    m_zoomMaxDistance{},
    m_zoomSoundVolumePercentAmount{}
{
}

// was originally INI::parseAudioSettingsDefinition
void AudioSettings::Parse_Audio_Settings_Definition(INI *ini)
{
    AudioSettings *as = g_theAudio->Get_Audio_Settings();
    ini->Init_From_INI(as, s_audioSettingsParseTable);
    OptionPreferences prefs;

    g_theAudio->Set_Preferred_3D_Provider(prefs.Get_Preferred_3D_Provider());
    g_theAudio->Set_Preferred_Speaker(prefs.Get_Speaker_Type());
    as->m_soundVolume = prefs.Get_Sound_Volume() / 100.0f;
    as->m_3dSoundVolume = prefs.Get_3DSound_Volume() / 100.0f;
    as->m_speechVolume = prefs.Get_Speech_Volume() / 100.0f;
    as->m_musicVolume = prefs.Get_Music_Volume() / 100.0f;
}
