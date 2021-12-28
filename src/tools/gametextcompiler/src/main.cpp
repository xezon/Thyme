/**
 * @file
 *
 * @author xezon
 *
 * @brief Game Text Compiler. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "log.h"
#include "processor.h"
#include <archivefilesystem.h>
#include <filesystem.h>
#include <localfilesystem.h>
#include <string>
#include <subsysteminterface.h>
#include <utility/arrayutil.h>
#include <win32bigfilesystem.h>
#include <win32localfilesystem.h>

using namespace Thyme;

// clang-format off
void Print_Help()
{
//       1         2         3         4         5         6         7         8         9        10        11        12
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
Print_Line(
R"#(// Function Command List
// Syntax: COMMAND_NAME(ARGUMENT_NAME_A:value,ARGUMENT_NAME_B:value)
// All capital words are interpreted keywords and must not be omitted.
// All symbols of ( : , ) are part of the syntax and must not be omitted.
// 'mandatory' and 'optional' words show whether or not argument is mandatory.
// [1] and [n] words show that argument takes one or multiple values.
// Space character will end current Command and begin new Command in command line.
// Commands are executed in the order they are written in the command line.

LOAD_CSF(FILE_ID:optional,FILE_PATH:mandatory)
                                    // Loads a CSF file from FILE_PATH into FILE_ID slot.
                                    // File language is set to the one stored in CSF file.

LOAD_STR(FILE_ID:optional,FILE_PATH:mandatory)
                                    // Loads a STR file from FILE_PATH into FILE_ID slot.
                                    // File language is not changed.

LOAD_MULTI_STR(FILE_ID:optional,FILE_PATH:mandatory,LANGUAGE:[n]mandatory)
                                    // Loads a Multi STR file from FILE_PATH
                                    // with LANGUAGE into FILE_ID slot.
                                    // File language is set to the first loaded language.

SAVE_CSF(FILE_ID:optional,FILE_PATH:mandatory)
                                    // Saves a CSF file to FILE_PATH from FILE_ID slot.

SAVE_STR(FILE_ID:optional,FILE_PATH:mandatory)
                                    // Saves a STR file to FILE_PATH from FILE_ID slot.

SAVE_MULTI_STR(FILE_ID:optional,FILE_PATH:mandatory,LANGUAGE:[n]mandatory)
                                    // Saves a Multi STR file to FILE_PATH
                                    // with LANGUAGE from FILE_ID slot.

UNLOAD(FILE_ID:optional,LANGUAGE:[n]optional)
                                    // Unloads string data from FILE_ID slot.
                                    // Uses the optionally specified language(s),
                                    // otherwise the current selected file language.

RESET(FILE_ID:optional)
                                    // Resets all string data.

MERGE_AND_OVERWRITE(FILE_ID:mandatory,FILE_ID:mandatory,LANGUAGE:[n]optional)
                                    // Merges and overwrites string data
                                    // in 1st FILE_ID from 2nd FILE_ID.
                                    // Uses the optionally specified language(s),
                                    // otherwise the current selected file language.

SET_OPTIONS(FILE_ID:optional,OPTION:[n]optional)
                                    // Sets options of OPTION in FILE_ID.

SET_LANGUAGE(FILE_ID:optional,LANGUAGE:[1]mandatory)
                                    // Sets language of LANGUAGE in FILE_ID.

SWAP_LANGUAGE_STRINGS(FILE_ID:optional,LANGUAGE:[1]mandatory,LANGUAGE:[1]mandatory)
                                    // Swaps string data in FILE_ID
                                    // between 1st LANGUAGE and 2nd LANGUAGE.

SWAP_AND_SET_LANGUAGE(FILE_ID:optional,LANGUAGE:[1]mandatory)
                                    // Swaps string data in FILE_ID between
                                    // current selected file language and LANGUAGE.
)#");
//       1         2         3         4         5         6         7         8         9        10        11        12
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
Print_Line(
R"#(// Command Argument List

FILE_ID:Number
                                    // FILE_ID takes number and allows to manage
                                    // multiple files in compiler. Default is 0.

FILE_PATH:Path
                                    // FILE_PATH takes any relative or absolute path.

LANGUAGE:All|English|German|French|
Spanish|Italian|Japanese|Korean|
Chinese|Brazilian|Polish|Unknown|
Russian|Arabic
                                    // LANGUAGE takes one [1] or multiple [n] languages.

OPTION:None|
Check_Buffer_Length_On_Load|
Check_Buffer_Length_On_Save|
Keep_Spaces_On_Load|
Print_Linebreaks_On_STR_Save|
Optimize_Memory_Size
                                    // OPTION takes one [1] or multiple [n] options.
)#");
//       1         2         3         4         5         6         7         8         9        10        11        12
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
Print_Line(
R"#(// Simplified Command List
// All capital words are interpreted keywords and must not be omitted.
// Commands are executed in the order they are listed here.

-OPTIONS option[n]
                                    // Sets option(s) for loaded and saved file.

-LOAD_CSF filepath.csf
                                    // Loads a CSF file from given file path.
                                    // File language is set to the one stored in CSF file.

-LOAD_STR filepath.str
                                    // Loads a STR file from given file path.
                                    // File language is not changed.

-LOAD_STR_LANGUAGES language[n]
                                    // Sets language(s) to load Multi STR file with.
                                    // File language is set to the first loaded language.

-SWAP_AND_SET_LANGUAGE language[1]
                                    // Swaps language strings and sets file language from
                                    // current file language to the given language.

-SAVE_CSF filepath.csf
                                    // Saves a CSF file to given file path.

-SAVE_STR filepath.str
                                    // Saves a STR file to given file path.

-SAVE_STR_LANGUAGES language[n]
                                    // Sets language(s) to save Multi STR file with.
)#");
}
// clang-format on

void Print_Error(const Processor::Result &result, const Processor::CommandTexts &command_texts)
{
    const size_t command_index = result.error_command_index;
    const char *result_name = Processor::Get_Result_Name(result.id);
    const char *command_name = (command_index < command_texts.size()) ? command_texts[command_index] : "";
    const std::string error_str(result.error_text.begin(), result.error_text.end());

    Print_Line("Execution stopped with error '%s' at command '%s' (%zu) and error string '%s'",
        result_name,
        command_name,
        command_index,
        error_str.c_str());
}

namespace
{
constexpr int NoError = 0;
constexpr int MissingArgumentsError = 1;
constexpr int ProcessorParseError = 2;
constexpr int ProcessorExecuteError = 3;
} // namespace

LocalFileSystem *Create_Local_File_System()
{
    return new Win32LocalFileSystem;
}

ArchiveFileSystem *Create_Archive_File_System()
{
    return new Win32BIGFileSystem;
}

class EngineSystemsCreator
{
public:
    EngineSystemsCreator()
    {
        g_theSubsystemList = new SubsystemInterfaceList;
        g_theFileSystem = new FileSystem;
        Init_Subsystem(g_theLocalFileSystem, "TheLocalFileSystem", Create_Local_File_System());
        g_theLocalFileSystem->Init();
#if 0
        Init_Subsystem(g_theArchiveFileSystem, "TheArchiveFileSystem", Create_Archive_File_System());
        g_theArchiveFileSystem->Init();
#endif
    }
    ~EngineSystemsCreator()
    {
        delete g_theArchiveFileSystem;
        delete g_theLocalFileSystem;
        delete g_theFileSystem;
        delete g_theSubsystemList;
        g_theArchiveFileSystem = nullptr;
        g_theLocalFileSystem = nullptr;
        g_theFileSystem = nullptr;
        g_theSubsystemList = nullptr;
    }
};

int main(int argc, const char *argv[])
{
    Print_Line("Thyme Game Text Compiler 1.0");
    Print_Line("============================");

    if (argc < 2) {
        Print_Help();
        return MissingArgumentsError;
    }

    GameTextFile::Set_Log_File(stderr);
    EngineSystemsCreator engine_systems;

    const auto command_texts = rts::Make_Array_View(argv + 1, argc - 1);
    Processor processor;
    Processor::Result result = processor.Parse_Commands(command_texts);

    if (result.id != Processor::ResultId::SUCCESS) {
        Print_Line("ERROR : Game Text Compiler failed to parse commands");
        Print_Error(result, command_texts);
        return ProcessorParseError;
    }

    result = processor.Execute_Commands();

    if (result.id != Processor::ResultId::SUCCESS) {
        Print_Line("ERROR : Game Text Compiler failed to execute commands");
        Print_Error(result, command_texts);
        return ProcessorExecuteError;
    }

    Print_Line("Game Text Compiler completed successfully");
    return NoError;
}
