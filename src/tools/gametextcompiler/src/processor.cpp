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
#include <utility/stringutil.h>

namespace Thyme
{
const Processor::FileId Processor::s_defaultFileId = { 0 };

Processor::Processor() : m_fileMap(), m_commands() {}

Processor::Result Processor::Prepare(rts::array_view<const char *> commands)
{
    Result result = Result::SUCCESS;

    for (const char *command : commands) {
        CommandAction action;

        result = Parse_Command(command, action);

        if (result != Result::SUCCESS) {
            break;
        }

        result = Add_New_Command(action, m_fileMap, m_commands);

        if (result != Result::SUCCESS) {
            break;
        }
    }
    return result;
}

Processor::Result Processor::Execute_Commands()
{
    Result result = Result::SUCCESS;

    for (CommandPtr &command : m_commands) {
        if (!command->Execute()) {
            result = Result::FAILURE;
            break;
        }
    }
    return result;
}

Processor::Result Processor::Parse_Command(const char *command, CommandAction &action)
{
    Result result = Result::SUCCESS;
    CommandActionId action_id = CommandActionId::INVALID;
    CommandArgumentId argument_id = CommandArgumentId::INVALID;
    CommandArguments arguments;
    const char *word_begin = command;
    const char *reader = command;

    while (*reader != '\0') {

        // Determine Command Action Type
        if (word_begin == command && *reader == '(') {
            const std::string word(word_begin, reader);
            if (!String_To_Command_Action(word.c_str(), action_id)) {
                result = Result::INVALID_COMMAND_ACTION;
                break;
            }
            word_begin = reader + 1;
        }
        // Determine Command Argument Type
        else if (argument_id == CommandArgumentId::INVALID && *reader == ':') {
            const std::string word(word_begin, reader);
            if (!String_To_Command_Argument(word.c_str(), argument_id)) {
                result = Result::INVALID_COMMAND_ARGUMENT;
                break;
            }
            arguments.emplace_back();
            word_begin = reader + 1;
        }
        // Determine Command Argument Values(s)
        else if (argument_id != CommandArgumentId::INVALID && (*reader == ',' || *reader == '|' || *reader == ')')) {
            std::string word(word_begin, reader);
            auto &variant = arguments.back().value;

            if (argument_id == CommandArgumentId::FILE_ID) {
                FileId file_id;
                file_id.value = std::stoi(word);
                variant.emplace<FileId>(std::move(file_id));

            } else if (argument_id == CommandArgumentId::FILE_PATH) {
                rts::Strip_Characters(word.data(), "\"");
                FilePath file_path;
                file_path.value = word.c_str();
                variant.emplace<FilePath>(std::move(file_path));

            } else if (argument_id == CommandArgumentId::LANGUAGES) {
                Languages languages;
                LanguageID language;
                if (!Name_To_Language(word.c_str(), language)) {
                    result = Result::INVALID_LANGUAGE_VALUE;
                    break;
                }
                const Languages *languages_ptr = std::get_if<Languages>(&variant);
                if (languages_ptr != nullptr) {
                    languages = *languages_ptr;
                }
                languages |= language;
                variant.emplace<Languages>(std::move(languages));

            } else if (argument_id == CommandArgumentId::OPTIONS) {
                GameTextOptions options;
                GameTextOption option;
                if (!Name_To_Game_Text_Option(word.c_str(), option)) {
                    result = Result::INVALID_LANGUAGE_VALUE;
                    break;
                }
                const GameTextOptions *options_ptr = std::get_if<GameTextOptions>(&variant);
                if (options_ptr != nullptr) {
                    options = *options_ptr;
                }
                options |= option;
                variant.emplace<GameTextOptions>(std::move(options));

            } else {
                captainslog_dbgassert(false, "CommandArgument is not defined");
            }

            if (*reader == ',') {
                argument_id = CommandArgumentId::INVALID;
            }
            word_begin = reader + 1;
        }

        ++reader;
    }

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

    return result;
}

Processor::Result Processor::Add_New_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands)
{
    Result result = Result::SUCCESS;

    Populate_File_Map(action, file_map);

    switch (action.action_id) {
        case CommandActionId::LOAD:
            result = Add_Load_Command(action, file_map, commands);
            break;
        case CommandActionId::LOAD_CSF:
            result = Add_Load_CSF_Command(action, file_map, commands);
            break;
        case CommandActionId::LOAD_STR:
            result = Add_Load_STR_Command(action, file_map, commands);
            break;
        case CommandActionId::SAVE:
            result = Add_Save_Command(action, file_map, commands);
            break;
        case CommandActionId::SAVE_CSF:
            result = Add_Save_CSF_Command(action, file_map, commands);
            break;
        case CommandActionId::SAVE_STR:
            result = Add_Save_STR_Command(action, file_map, commands);
            break;
        case CommandActionId::UNLOAD:
            result = Add_Unload_Command(action, file_map, commands);
            break;
        case CommandActionId::RESET:
            result = Add_Reset_Command(action, file_map, commands);
            break;
        case CommandActionId::MERGE_AND_OVERWRITE:
            result = Add_Merge_Command(action, file_map, commands);
            break;
        case CommandActionId::SET_OPTIONS:
            result = Add_Set_Options_Command(action, file_map, commands);
            break;
        case CommandActionId::SET_LANGUAGE:
            result = Add_Set_Language_Command(action, file_map, commands);
            break;
        case CommandActionId::SWAP_LANGUAGE_STRINGS:
            result = Add_Swap_Language_Command(action, file_map, commands);
            break;
        default:
            captainslog_dbgassert(false, "CommandAction is not defined");
            break;
    }

    return result;
}

Processor::Result Processor::Add_Load_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto file_path = Get_File_Path(action.arguments);

    if (file_path == nullptr) {
        return Result::MISSING_FILE_PATH_ARGUMENT;
    }

    commands.emplace_back(new LoadCommand(file_ptr, file_path));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Load_CSF_Command(
    const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands)
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
    const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands)
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

Processor::Result Processor::Add_Save_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto file_path = Get_File_Path(action.arguments);

    if (file_path == nullptr) {
        return Result::MISSING_FILE_PATH_ARGUMENT;
    }

    commands.emplace_back(new SaveCommand(file_ptr, file_path));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Save_CSF_Command(
    const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto file_path = Get_File_Path(action.arguments);
    const auto language = Get_Language(action.arguments);

    if (file_path == nullptr) {
        return Result::MISSING_FILE_PATH_ARGUMENT;
    }

    commands.emplace_back(new SaveCsfCommand(file_ptr, file_path, language));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Save_STR_Command(
    const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands)
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

Processor::Result Processor::Add_Unload_Command(
    const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto languages = Get_Languages(action.arguments);

    commands.emplace_back(new UnloadCommand(file_ptr, languages));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Reset_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);

    commands.emplace_back(new ResetCommand(file_ptr));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Merge_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands)
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
    const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands)
{
    const auto file_ptr = Get_File_Ptr(action.arguments, file_map);
    const auto options = Get_Options(action.arguments);

    commands.emplace_back(new SetOptionsCommand(file_ptr, options));
    return Result::SUCCESS;
}

Processor::Result Processor::Add_Set_Language_Command(
    const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands)
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
    const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands)
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

void Processor::Populate_File_Map(const CommandAction &action, GameTextFileMap &file_map)
{
    const FileId *file_id_ptr = Find_Ptr<FileId>(action.arguments);
    const FileId file_id = (file_id_ptr == nullptr) ? s_defaultFileId : *file_id_ptr;
    Populate_File_Map(file_id, file_map);
}

void Processor::Populate_File_Map(FileId file_id, GameTextFileMap &file_map)
{
    const GameTextFileMap::iterator it = file_map.find(file_id);
    if (it == file_map.end()) {
        file_map.emplace(file_id, GameTextFilePtr(new GameTextFile()));
    }
}

GameTextFilePtr Processor::Get_File_Ptr(FileId file_id, GameTextFileMap &file_map)
{
    const GameTextFileMap::iterator it = file_map.find(file_id);
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

GameTextFilePtr Processor::Get_File_Ptr(const CommandArguments &arguments, GameTextFileMap &file_map, size_t occurence)
{
    const FileId *ptr = Find_Ptr<FileId>(arguments, occurence);
    const FileId file_id = (ptr == nullptr) ? s_defaultFileId : *ptr;
    return Get_File_Ptr(file_id, file_map);
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
