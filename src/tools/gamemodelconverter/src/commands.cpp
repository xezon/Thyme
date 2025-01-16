/**
 * @file
 *
 * @author xezon, DevGeniusCode
 *
 * @brief Model Converter Commands. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "commands.h"

namespace Thyme
{
namespace
{
const char *const s_command_action_names[] = {
    "LOAD_W3D",
    "LOAD_W3X",
    "LOAD_BLEND",
    "LOAD_MAX",
    "SAVE_W3D",
    "SAVE_W3X",
    "SAVE_BLEND",
    "SAVE_MAX",
    "RESET",
    "SET_OPTIONS",
};

const char *const s_command_argument_names[] = {
    "FILE_ID",
    "FILE_PATH",
    "OPTION",
};

const char *const s_simple_action_names[] = {
    "OPTIONS",
    "LOAD_W3D",
    "LOAD_W3X",
    "LOAD_BLEND",
    "LOAD_MAX",
    "SAVE_W3D",
    "SAVE_W3X",
    "SAVE_BLEND",
    "SAVE_MAX",

};

static_assert(ARRAY_SIZE(s_command_action_names) == g_commandActionCount);
static_assert(ARRAY_SIZE(s_command_argument_names) == g_commandArgumentCount);
static_assert(ARRAY_SIZE(s_simple_action_names) == g_simpleActionCount);

template<typename EnumType, size_t Size>
bool String_To_Enum_Id(const char *str, EnumType &id, const char *const (&search_names)[Size])
{
    for (size_t index = 0; index < ARRAY_SIZE(search_names); ++index) {
        if (strcasecmp(str, search_names[index]) == 0) {
            id = static_cast<EnumType>(index);
            return true;
        }
    }
    return false;
}
} // namespace

bool String_To_Command_Action_Id(const char *str, CommandActionId &action_id)
{
    return String_To_Enum_Id(str, action_id, s_command_action_names);
}

bool String_To_Command_Argument_Id(const char *str, CommandArgumentId &argument_id)
{
    return String_To_Enum_Id(str, argument_id, s_command_argument_names);
}

bool String_To_Simple_Action_Id(const char *str, SimpleActionId &action_id)
{
    return String_To_Enum_Id(str, action_id, s_simple_action_names);
}

CommandId Command::s_id = 1000000000;

bool LoadW3DCommand::Execute() const
{
    return m_filePtr->Load_CSF(m_filePath.c_str());
}

bool LoadW3XCommand::Execute() const
{
    return m_filePtr->Load_W3X(m_filePath.c_str());
}

bool LoadBlendCommand::Execute() const
{
    return m_filePtr->Load_BLEND(m_filePath.c_str());
}

bool LoadMaxCommand::Execute() const
{
    return m_filePtr->Load_MAX(m_filePath.c_str());
}

bool SaveW3DCommand::Execute() const
{
    return m_filePtr->Save_W3D(m_filePath.c_str());
}

bool SaveW3XCommand::Execute() const
{
    return m_filePtr->Save_W3X(m_filePath.c_str());
}

bool SaveBlendCommand::Execute() const
{
    return m_filePtr->Save_BLEND(m_filePath.c_str());
}

bool SaveMaxCommand::Execute() const
{
    return m_filePtr->Save_MAX(m_filePath.c_str());
}

bool ResetCommand::Execute() const
{
    m_filePtr->Reset();
    return true;
}

bool SetOptionsCommand::Execute() const
{
    m_filePtr->Set_Options(m_options);
    return true;
}

} // namespace Thyme
