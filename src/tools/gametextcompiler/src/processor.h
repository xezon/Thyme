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
#pragma once

#include "commands.h"
#include <unordered_map>
#include <utility/arrayview.h>
#include <variant>
#include <vector>

namespace Thyme
{
class Processor
{
public:
    enum class Result
    {
        SUCCESS,
        FAILURE,
        INVALID_COMMAND_ACTION,
        INVALID_COMMAND_ARGUMENT,
        INVALID_LANGUAGE_VALUE,
        INVALID_OPTION_VALUE,
        INVALID_FILE_ID_ARGUMENT,
        MISSING_FILE_PATH_ARGUMENT,
        MISSING_LANGUAGE_ARGUMENT,
    };

public:
    Processor();

    Result Prepare(rts::array_view<const char *> commands);
    Result Execute_Commands();

private:
    using CommandPtr = std::shared_ptr<Command>;
    using CommandPtrs = std::vector<CommandPtr>;

    struct FileId
    {
        bool operator==(const FileId &other) const { return value == other.value; }
        int value;
    };

    struct FileIdHash
    {
        std::size_t operator()(const FileId &key) const { return std::hash<int>{}(key.value); }
    };

    using GameTextFileMap = std::unordered_map<FileId, GameTextFilePtr, FileIdHash>;

    struct FilePath
    {
        std::string value;
    };

    struct CommandArgument
    {
        std::variant<FileId, FilePath, Languages, GameTextOptions> value;
    };

    using CommandArguments = std::vector<CommandArgument>;

    struct CommandAction
    {
        CommandActionId action_id;
        CommandArguments arguments;
    };

private:
    static Result Parse_Command(const char *command, CommandAction &action);

    static Result Add_New_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands);
    static Result Add_Load_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands);
    static Result Add_Load_CSF_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands);
    static Result Add_Load_STR_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands);
    static Result Add_Save_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands);
    static Result Add_Save_CSF_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands);
    static Result Add_Save_STR_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands);
    static Result Add_Unload_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands);
    static Result Add_Reset_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands);
    static Result Add_Merge_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands);
    static Result Add_Set_Options_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands);
    static Result Add_Set_Language_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands);
    static Result Add_Swap_Language_Command(const CommandAction &action, GameTextFileMap &file_map, CommandPtrs &commands);

    static void Populate_File_Map(const CommandAction &action, GameTextFileMap &file_map);
    static void Populate_File_Map(FileId file_id, GameTextFileMap &file_map);
    static GameTextFilePtr Get_File_Ptr(FileId file_id, GameTextFileMap &file_map);

    template<class Type> static const Type *Find_Ptr(const CommandArguments &arguments, size_t occurence = 0);
    static GameTextFilePtr Get_File_Ptr(const CommandArguments &arguments, GameTextFileMap &file_map, size_t occurence = 0);
    static const char *Get_File_Path(const CommandArguments &arguments, size_t occurence = 0);
    static Languages Get_Languages(const CommandArguments &arguments, size_t occurence = 0);
    static LanguageID Get_Language(const CommandArguments &arguments, size_t occurence = 0);
    static GameTextOptions Get_Options(const CommandArguments &arguments, size_t occurence = 0);

private:
    GameTextFileMap m_fileMap;
    CommandPtrs m_commands;

    static const FileId s_defaultFileId;
};

} // namespace Thyme
