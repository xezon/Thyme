/**
 * @file
 *
 * @author xezon
 *
 * @brief Game Text Compiler Commands
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
const char *s_command_action_names[] = {
    "LOAD",
    "LOAD_CSF",
    "LOAD_STR",
    "SAVE",
    "SAVE_CSF",
    "SAVE_STR",
    "UNLOAD",
    "RESET",
    "MERGE_AND_OVERWRITE",
    "SET_OPTIONS",
    "SET_LANGUAGE",
    "SWAP_LANGUAGE_STRINGS",
};

const char *s_command_argument_names[] = {
    "FILE_ID",
    "FILE_PATH",
    "LANGUAGE",
    "OPTION",
};
} // namespace

bool String_To_Command_Action(const char *str, CommandActionId &action_id)
{
    for (size_t index = 0; index < ARRAY_SIZE(s_command_action_names); ++index) {
        if (strcasecmp(str, s_command_action_names[index]) == 0) {
            action_id = static_cast<CommandActionId>(index);
            return true;
        }
    }
    return false;
}

bool String_To_Command_Argument(const char *str, CommandArgumentId &argument_id)
{
    for (size_t index = 0; index < ARRAY_SIZE(s_command_argument_names); ++index) {
        if (strcasecmp(str, s_command_argument_names[index]) == 0) {
            argument_id = static_cast<CommandArgumentId>(index);
            return true;
        }
    }
    return false;
}

CommandId Command::s_id = 0;

} // namespace Thyme
