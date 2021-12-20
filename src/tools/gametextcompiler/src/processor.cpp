/**
 * @file
 *
 * @author xezon
 *
 * @brief Game Text Compiler Processor
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "processor.h"
#include "commands.h"
#include <algorithm>
#include <utility/stringutil.h>

namespace Thyme
{

const Processor::FileId Processor::s_defaultFileId = { 0 };

Processor::Processor() : m_fileMap(), m_commands() {}

Processor::Result Processor::Parse_Commands(rts::array_view<const char *> commands)
{
    if (Has_Simple_Command(commands)) {
        return Parse_Simple_Commands(commands);
    } else {
        return Parse_Function_Commands(commands);
    }
}

Processor::Result Processor::Execute_Commands() const
{
    Result result = Result::SUCCESS;

    for (const CommandPtr &command : m_commands) {
        if (!command->Execute()) {
            result = Result::FAILURE;
            break;
        }
    }
    return result;
}

Processor::Result Processor::Parse_Function_Commands(rts::array_view<const char *> commands)
{
    Result result = Result::SUCCESS;

    for (const char *command : commands) {

        CommandAction action;

        result = Parse_Function_Command(action, command);

        if (result != Result::SUCCESS) {
            break;
        }

        result = Add_New_Command(m_commands, m_fileMap, action);

        if (result != Result::SUCCESS) {
            break;
        }
    }
    return result;
}

Processor::Result Processor::Parse_Simple_Commands(rts::array_view<const char *> commands)
{
    Result result = Result::SUCCESS;
    SimpleCommandActions actions;

    if (!commands.empty()) {

        const size_t size = commands.size();

        for (size_t index = 0; index < size - 1; ++index) {

            if (!Is_Simple_Command(commands[index])) {
                continue;
            }

            const char *command_name = commands[index] + 1;
            const char *command_value = commands[index + 1];

            result = Parse_Simple_Command(actions, command_name, command_value);

            if (result != Result::SUCCESS) {
                break;
            }
        }
    }

    {
        // Remove set language commands here if STR is loaded or saved because it is not necessary.

        if (actions[size_t(SimpleSequenceId::LOAD)].action_id == CommandActionId::LOAD_STR) {
            actions[size_t(SimpleSequenceId::LOAD_SET_LANGUAGE)].action_id = CommandActionId::INVALID;
        }

        if (actions[size_t(SimpleSequenceId::SAVE)].action_id == CommandActionId::SAVE_STR) {
            actions[size_t(SimpleSequenceId::SAVE_SET_LANGUAGE)].action_id = CommandActionId::INVALID;
        }
    }

    for (const CommandAction &action : actions) {
        if (action.action_id != CommandActionId::INVALID) {
            result = Add_New_Command(m_commands, m_fileMap, action);
            if (result != Result::SUCCESS) {
                break;
            }
        }
    }

    return result;
}

bool Processor::Has_Simple_Command(rts::array_view<const char *> commands)
{
    for (const char *command : commands) {
        if (Is_Simple_Command(command)) {
            return true;
        }
    }
    return false;
}

bool Processor::Is_Simple_Command(const char *command)
{
    return command != nullptr && *command == '-';
}

template<size_t Size> bool Processor::Parse_Next_Word(std::string &word, const char *&str, const char (&separators)[Size])
{
    const char *reader = str;

    do {
        for (const char separator : separators) {
            if (*reader == separator && reader > str) {
                word.assign(str, reader);
                str = reader;
                if (*str != '\0') {
                    str += 1;
                }
                return true;
            }
        }
    } while (*reader++ != '\0');

    return false;
}

Processor::Result Processor::Parse_Function_Command(CommandAction &action, const char *command)
{
    Result result = Result::SUCCESS;
    CommandActionId action_id = CommandActionId::INVALID;
    CommandArgumentId argument_id = CommandArgumentId::INVALID;
    CommandArguments arguments;
    const char *reader = command;
    std::string word;

    while (true) {
        if (reader == command && Parse_Next_Word(word, reader, { '(' })) {
            // Determine Command Action Type
            if (!String_To_Command_Action_Id(word.c_str(), action_id)) {
                result = Result::INVALID_COMMAND_ACTION;
                break;
            }

        } else if (argument_id == CommandArgumentId::INVALID && Parse_Next_Word(word, reader, { ':' })) {
            // Determine Command Argument Type
            if (!String_To_Command_Argument_Id(word.c_str(), argument_id)) {
                result = Result::INVALID_COMMAND_ARGUMENT;
                break;
            }
            arguments.emplace_back();

        } else if (argument_id != CommandArgumentId::INVALID && Parse_Next_Word(word, reader, { ',', '|', ')' })) {
            // Determine Command Argument Values(s)
            result = Parse_Command_Argument(arguments.back(), word, argument_id);
            if (result != Result::SUCCESS) {
                break;
            }
            if (*(reader - 1) == ',') {
                argument_id = CommandArgumentId::INVALID;
            }

        } else {
            break;
        }
    }

    if (result == Result::SUCCESS) {
        if (action_id == CommandActionId::INVALID) {
            result = Result::INVALID_COMMAND_ACTION;
        }
        if (arguments.empty()) {
            result = Result::INVALID_COMMAND_ARGUMENT;
        }
        if (result == Result::SUCCESS) {
            action.action_id = action_id;
            action.arguments.swap(arguments);
        }
    }

    return result;
}

Processor::Result Processor::Parse_Simple_Command(
    SimpleCommandActions &actions, const char *command_name, const char *command_value)
{
    SimpleActionId simple_action_id;

    if (!String_To_Simple_Action_Id(command_name, simple_action_id)) {
        return Result::INVALID_COMMAND_ACTION;
    }

    const size_t sequence_count = 2;
    SimpleSequenceId sequence_ids[sequence_count];
    CommandActionId action_ids[sequence_count];
    CommandArgumentId argument_id = CommandArgumentId::INVALID;
    std::fill(sequence_ids, sequence_ids + sequence_count, SimpleSequenceId::INVALID);
    std::fill(action_ids, action_ids + sequence_count, CommandActionId::INVALID);

    switch (simple_action_id) {
        case SimpleActionId::OPTIONS:
            sequence_ids[0] = SimpleSequenceId::SET_OPTIONS;
            action_ids[0] = CommandActionId::SET_OPTIONS;
            argument_id = CommandArgumentId::OPTIONS;
            break;
        case SimpleActionId::LOAD_LANGUAGES:
            sequence_ids[0] = SimpleSequenceId::LOAD_SET_LANGUAGE;
            sequence_ids[1] = SimpleSequenceId::LOAD;
            action_ids[0] = CommandActionId::SWAP_AND_SET_LANGUAGE;
            action_ids[1] = CommandActionId::LOAD_STR;
            argument_id = CommandArgumentId::LANGUAGES;
            break;
        case SimpleActionId::LOAD_CSF_FILE:
            sequence_ids[0] = SimpleSequenceId::LOAD;
            action_ids[0] = CommandActionId::LOAD_CSF;
            argument_id = CommandArgumentId::FILE_PATH;
            break;
        case SimpleActionId::LOAD_STR_FILE:
            sequence_ids[0] = SimpleSequenceId::LOAD;
            action_ids[0] = CommandActionId::LOAD_STR;
            argument_id = CommandArgumentId::FILE_PATH;
            break;
        case SimpleActionId::SAVE_LANGUAGES:
            sequence_ids[0] = SimpleSequenceId::SAVE_SET_LANGUAGE;
            sequence_ids[1] = SimpleSequenceId::SAVE;
            action_ids[0] = CommandActionId::SWAP_AND_SET_LANGUAGE;
            action_ids[1] = CommandActionId::SAVE_STR;
            argument_id = CommandArgumentId::LANGUAGES;
            break;
        case SimpleActionId::SAVE_CSF:
            sequence_ids[0] = SimpleSequenceId::SAVE;
            action_ids[0] = CommandActionId::SAVE_CSF;
            argument_id = CommandArgumentId::FILE_PATH;
            break;
        case SimpleActionId::SAVE_STR:
            sequence_ids[0] = SimpleSequenceId::SAVE;
            action_ids[0] = CommandActionId::SAVE_STR;
            argument_id = CommandArgumentId::FILE_PATH;
            break;
    }

    static_assert(g_simpleActionCount == 7, "SimpleAction is missing");

    Result result = Result::SUCCESS;
    CommandArgument argument;

    {
        const char *reader = command_value;
        std::string word;

        while (Parse_Next_Word(word, reader, { '|', '\0' })) {
            result = Parse_Command_Argument(argument, word, argument_id);
            if (result != Result::SUCCESS) {
                break;
            }
        }
    }

    for (size_t i = 0; i < sequence_count; ++i) {
        if (sequence_ids[i] != SimpleSequenceId::INVALID) {
            CommandAction &action = actions[size_t(sequence_ids[i])];
            action.action_id = action_ids[i];
            action.arguments.push_back(argument);
        }
    }

    return result;
}

Processor::Result Processor::Parse_Command_Argument(
    CommandArgument &argument, std::string &str, CommandArgumentId argument_id)
{
    Result result = Result::SUCCESS;

    switch (argument_id) {
        case CommandArgumentId::FILE_ID: {
            FileId file_id;
            file_id.value = std::stoi(str);
            argument.value.emplace<FileId>(std::move(file_id));
            break;
        }
        case CommandArgumentId::FILE_PATH: {
            rts::Strip_Characters(str.data(), "\"");
            FilePath file_path;
            file_path.value = str.c_str();
            argument.value.emplace<FilePath>(std::move(file_path));
            break;
        }
        case CommandArgumentId::LANGUAGES: {
            Languages languages;
            LanguageID language;
            if (!Name_To_Language(str.c_str(), language)) {
                result = Result::INVALID_LANGUAGE_VALUE;
                break;
            }
            const Languages *languages_ptr = std::get_if<Languages>(&argument.value);
            if (languages_ptr != nullptr) {
                languages = *languages_ptr;
            }
            languages |= language;
            argument.value.emplace<Languages>(std::move(languages));
            break;
        }
        case CommandArgumentId::OPTIONS: {
            GameTextOptions options;
            GameTextOption option;
            if (!Name_To_Game_Text_Option(str.c_str(), option)) {
                result = Result::INVALID_OPTION_VALUE;
                break;
            }
            const GameTextOptions *options_ptr = std::get_if<GameTextOptions>(&argument.value);
            if (options_ptr != nullptr) {
                options = *options_ptr;
            }
            options |= option;
            argument.value.emplace<GameTextOptions>(std::move(options));
            break;
        }
    }

    static_assert(g_commandArgumentCount == 4, "CommandArgument is missing");

    return result;
}

Processor::Result Processor::Add_New_Command(CommandPtrs &commands, FileMap &file_map, const CommandAction &action)
{
    Result result = Result::SUCCESS;

    Populate_File_Map(file_map, action);

    switch (action.action_id) {
        case CommandActionId::LOAD_CSF:
            result = Add_Load_CSF_Command(commands, file_map, action);
            break;
        case CommandActionId::LOAD_STR:
            result = Add_Load_STR_Command(commands, file_map, action);
            break;
        case CommandActionId::SAVE_CSF:
            result = Add_Save_CSF_Command(commands, file_map, action);
            break;
        case CommandActionId::SAVE_STR:
            result = Add_Save_STR_Command(commands, file_map, action);
            break;
        case CommandActionId::UNLOAD:
            result = Add_Unload_Command(commands, file_map, action);
            break;
        case CommandActionId::RESET:
            result = Add_Reset_Command(commands, file_map, action);
            break;
        case CommandActionId::MERGE_AND_OVERWRITE:
            result = Add_Merge_Command(commands, file_map, action);
            break;
        case CommandActionId::SET_OPTIONS:
            result = Add_Set_Options_Command(commands, file_map, action);
            break;
        case CommandActionId::SET_LANGUAGE:
            result = Add_Set_Language_Command(commands, file_map, action);
            break;
        case CommandActionId::SWAP_LANGUAGE_STRINGS:
            result = Add_Swap_Language_Command(commands, file_map, action);
            break;
        case CommandActionId::SWAP_AND_SET_LANGUAGE:
            result = Add_Set_And_Swap_Language_Command(commands, file_map, action);
            break;
    }

    static_assert(g_commandActionCount == 11, "CommandAction is missing");

    return result;
}

Processor::Result Processor::Add_Load_CSF_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto file_path = Get_File_Path(action.arguments);

    if (file_path == nullptr) {
        return Result::MISSING_FILE_PATH_ARGUMENT;
    }

    commands.emplace_back(new LoadCsfCommand(file_ptr, file_path));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Load_STR_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto file_path = Get_File_Path(action.arguments);
    const auto languages = Get_Languages(action.arguments);

    if (file_path == nullptr) {
        return Result::MISSING_FILE_PATH_ARGUMENT;
    }

    commands.emplace_back(new LoadStrCommand(file_ptr, file_path, languages));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Save_CSF_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto file_path = Get_File_Path(action.arguments);

    if (file_path == nullptr) {
        return Result::MISSING_FILE_PATH_ARGUMENT;
    }

    commands.emplace_back(new SaveCsfCommand(file_ptr, file_path));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Save_STR_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto file_path = Get_File_Path(action.arguments);
    const auto languages = Get_Languages(action.arguments);

    if (file_path == nullptr) {
        return Result::MISSING_FILE_PATH_ARGUMENT;
    }

    commands.emplace_back(new SaveStrCommand(file_ptr, file_path, languages));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Unload_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto languages = Get_Languages(action.arguments);

    commands.emplace_back(new UnloadCommand(file_ptr, languages));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Reset_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);

    commands.emplace_back(new ResetCommand(file_ptr));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Merge_Command(CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr_a = Get_File_Ptr(action.arguments, file_map, 0);
    const auto file_ptr_b = Get_File_Ptr(action.arguments, file_map, 1);
    const auto languages = Get_Languages(action.arguments);

    if (file_ptr_a == file_ptr_b) {
        return Result::INVALID_FILE_ID_ARGUMENT;
    }

    commands.emplace_back(new MergeAndOverwriteCommand(file_ptr_a, file_ptr_b, languages));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Set_Options_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto options = Get_Options(action.arguments);

    commands.emplace_back(new SetOptionsCommand(file_ptr, options));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Set_Language_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto language = Get_Language(action.arguments);

    if (language == LanguageID::UNKNOWN) {
        return Result::MISSING_LANGUAGE_ARGUMENT;
    }

    commands.emplace_back(new SetLanguageCommand(file_ptr, language));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Swap_Language_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto language_a = Get_Language(action.arguments, 0);
    const auto language_b = Get_Language(action.arguments, 1);

    if (language_a == LanguageID::UNKNOWN) {
        return Result::MISSING_LANGUAGE_ARGUMENT;
    }
    if (language_b == LanguageID::UNKNOWN) {
        return Result::MISSING_LANGUAGE_ARGUMENT;
    }

    commands.emplace_back(new SwapLanguageStringsCommand(file_ptr, language_a, language_b));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Set_And_Swap_Language_Command(
    CommandPtrs &commands, const FileMap &file_map, const CommandAction &action)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto language = Get_Language(action.arguments, 0);

    if (language == LanguageID::UNKNOWN) {
        return Result::MISSING_LANGUAGE_ARGUMENT;
    }

    commands.emplace_back(new SwapAndSetLanguageCommand(file_ptr, language));
    return Result::SUCCESS;
}

void Processor::Populate_File_Map(FileMap &file_map, const CommandAction &action)
{
    const FileId *file_id_ptr = Find_Ptr<FileId>(action.arguments);
    const FileId file_id = (file_id_ptr == nullptr) ? s_defaultFileId : *file_id_ptr;
    Populate_File_Map(file_map, file_id);
}

void Processor::Populate_File_Map(FileMap &file_map, FileId file_id)
{
    const FileMap::iterator it = file_map.find(file_id);
    if (it == file_map.end()) {
        file_map.emplace(file_id, GameTextFilePtr(new GameTextFile()));
    }
}

GameTextFilePtr Processor::Get_File_Ptr(const FileMap &file_map, FileId file_id)
{
    const FileMap::const_iterator it = file_map.find(file_id);
    captainslog_assert(it != file_map.end());
    return it->second;
}

template<class Type> const Type *Processor::Find_Ptr(const CommandArguments &arguments, size_t occurence)
{
    size_t num = 0;
    for (const CommandArgument &argument : arguments) {
        const Type *type_ptr = std::get_if<Type>(&argument.value);
        if (type_ptr != nullptr) {
            if (occurence == num++) {
                return type_ptr;
            }
        }
    }
    return nullptr;
}

GameTextFilePtr Processor::Get_File_Ptr(const CommandArguments &arguments, const FileMap &file_map, size_t occurence)
{
    const FileId *ptr = Find_Ptr<FileId>(arguments, occurence);
    const FileId file_id = (ptr == nullptr) ? s_defaultFileId : *ptr;
    return Get_File_Ptr(file_map, file_id);
}

const char *Processor::Get_File_Path(const CommandArguments &arguments, size_t occurence)
{
    const FilePath *ptr = Find_Ptr<FilePath>(arguments, occurence);
    return (ptr == nullptr) ? nullptr : ptr->value.c_str();
}

Languages Processor::Get_Languages(const CommandArguments &arguments, size_t occurence)
{
    const Languages *ptr = Find_Ptr<Languages>(arguments, occurence);
    return (ptr == nullptr) ? Languages() : *ptr;
}

LanguageID Processor::Get_Language(const CommandArguments &arguments, size_t occurence)
{
    LanguageID language = LanguageID::UNKNOWN;
    const Languages *ptr = Find_Ptr<Languages>(arguments, occurence);
    if (ptr != nullptr) {
        ptr->get(language, 0);
    }
    return language;
}

GameTextOptions Processor::Get_Options(const CommandArguments &arguments, size_t occurence)
{
    const GameTextOptions *ptr = Find_Ptr<GameTextOptions>(arguments, occurence);
    return (ptr == nullptr) ? GameTextOptions() : *ptr;
}

} // namespace Thyme
