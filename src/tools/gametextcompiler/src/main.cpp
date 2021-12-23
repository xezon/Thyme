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
#include "processor.h"
#include <archivefilesystem.h>
#include <captainslog.h>
#include <filesystem.h>
#include <localfilesystem.h>
#include <string>
#include <subsysteminterface.h>
#include <utility/arrayutil.h>
#include <win32bigfilesystem.h>
#include <win32localfilesystem.h>

using namespace Thyme;

// clang-format off
void Log_Help()
{
//       1         2         3         4         5         6         7         8         9        10        11        12
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
captainslog_info(
    R"#(Function Command List.
          Syntax: COMMAND_NAME(ARGUMENT_NAME_A:value,ARGUMENT_NAME_B:value)
          All capital words are interpreted keywords and must not be omitted.
          All symbols of ( : , ) are part of the syntax and must not be omitted.
          'mandatory' and 'optional' words show whether or not argument is mandatory.
          [1] and [n] words show that argument takes one or multiple values.
          Space character will end current Command and begin new Command in command line.
          Commands are executed in the order they are written in the command line.

No 01 : LOAD_CSF(FILE_ID:optional,FILE_PATH:mandatory)
No 02 : LOAD_STR(FILE_ID:optional,FILE_PATH:mandatory)
No 03 : LOAD_MULTI_STR(FILE_ID:optional,FILE_PATH:mandatory,LANGUAGE:[n]mandatory)
No 04 : SAVE_CSF(FILE_ID:optional,FILE_PATH:mandatory)
No 05 : SAVE_STR(FILE_ID:optional,FILE_PATH:mandatory)
No 06 : SAVE_MULTI_STR(FILE_ID:optional,FILE_PATH:mandatory,LANGUAGE:[n]mandatory)
No 07 : UNLOAD(FILE_ID:optional,LANGUAGE:[n]optional)
No 08 : RESET(FILE_ID:optional)
No 09 : MERGE_AND_OVERWRITE(FILE_ID:mandatory,FILE_ID:mandatory,LANGUAGE:[n]optional)
No 10 : SET_OPTIONS(FILE_ID:optional,OPTION:[n]optional)
No 11 : SET_LANGUAGE(FILE_ID:optional,LANGUAGE:[1]mandatory)
No 12 : SWAP_LANGUAGE_STRINGS(FILE_ID:optional,LANGUAGE:[1]mandatory,LANGUAGE:[1]mandatory)
No 13 : SWAP_AND_SET_LANGUAGE(FILE_ID:optional,LANGUAGE:[1]mandatory)

.. 01 : Loads a CSF file from FILE_PATH into FILE_ID slot.
          File language is set to the one stored in CSF file.
.. 02 : Loads a STR file from FILE_PATH into FILE_ID slot.
          File language is not changed.
.. 03 : Loads a Multi STR file from FILE_PATH with LANGUAGE into FILE_ID slot.
          File language is set to the first loaded language.
.. 04 : Saves a CSF file to FILE_PATH from FILE_ID slot.
.. 05 : Saves a STR file to FILE_PATH from FILE_ID slot.
.. 06 : Saves a Multi STR file to FILE_PATH with LANGUAGE from FILE_ID slot.
.. 07 : Unloads string data of LANGUAGE from FILE_ID slot.
.. 08 : Resets all string data.
.. 09 : Merges and overwrites string data of LANGUAGE in 1st FILE_ID from 2nd FILE_ID.
.. 10 : Sets options of OPTION in FILE_ID.
.. 11 : Sets language of LANGUAGE in FILE_ID.
.. 12 : Swaps string data in FILE_ID between 1st LANGUAGE and 2nd LANGUAGE.
.. 13 : Swaps string data in FILE_ID between current selected file language and LANGUAGE.
)#");
//       1         2         3         4         5         6         7         8         9        10        11        12
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
captainslog_info(
    R"#(Command Argument List.

No 01 : FILE_ID:Number
No 02 : FILE_PATH:Path
No 03 : LANGUAGE:All|English|German|French|Spanish|Italian|Japanese|Korean|
                 Chinese|Brazilian|Polish|Unknown|Russian|Arabic
No 04 : OPTION:None|Check_Buffer_Length_On_Load|Check_Buffer_Length_On_Save|
               Keep_Spaces_On_STR_Load|Print_Linebreaks_On_STR_Save|Optimize_Memory_Size

.. 01 : FILE_ID takes number and allows to manage multiple files in compiler. Default is 0.
.. 02 : FILE_PATH takes any relative or absolute path.
.. 03 : LANGUAGE takes one [1] or multiple [n] languages.
.. 04 : OPTION takes one [1] or multiple [n] options.
)#");
//       1         2         3         4         5         6         7         8         9        10        11        12
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
captainslog_info(
    R"#(Simplified Command List.
          All capital words are interpreted keywords and must not be omitted.
          Commands are executed in the order they are listed here.

No 01 : -OPTIONS               option[n]
No 02 : -LOAD_CSF_FILE         filepath.csf
No 03 : -LOAD_STR_FILE         filepath.str
No 04 : -LOAD_STR_LANGUAGES    language[n]
No 05 : -SWAP_AND_SET_LANGUAGE language[1]
No 06 : -SAVE_CSF              filepath.csf
No 07 : -SAVE_STR              filepath.str
No 08 : -SAVE_STR_LANGUAGES    language[n]

.. 01 : Sets option(s) for loaded and saved file.
.. 02 : Loads a CSF file from given file path.
          File language is set to the one stored in CSF file.
.. 03 : Loads a STR file from given file path.
          File language is not changed.
.. 04 : Sets language(s) to load Multi STR file with.
          File language is set to the first loaded language.
.. 05 : Swaps language strings and sets file language from
          current file language to the given language.
.. 06 : Saves a CSF file to given file path.
.. 07 : Saves a STR file to given file path.
.. 08 : Sets language(s) to save Multi STR file with.
)#");
}
// clang-format on

void Log_Error(const Processor::Result &result, const Processor::CommandTexts &command_texts)
{
    const size_t command_index = result.error_command_index;
    const char *result_name = Processor::Get_Result_Name(result.id);
    const char *command_name = (command_index < command_texts.size()) ? command_texts[command_index] : "";
    const std::string error_str(result.error_text.begin(), result.error_text.end());

    captainslog_error("Execution stopped with error '%s' at command '%s' (%zu) and error string '%s'",
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

class CaptainsLogCreator
{
public:
    CaptainsLogCreator()
    {
        captains_settings_t captains_settings = { 0 };
        captains_settings.level = LOGLEVEL_DEBUG;
        captains_settings.console = true;
        captainslog_init(&captains_settings);
    }
    ~CaptainsLogCreator() { captainslog_deinit(); }
};

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
    CaptainsLogCreator captains_log;

    captainslog_info("Thyme Game Text Compiler 1.0");

    if (argc < 2) {
        Log_Help();
        return MissingArgumentsError;
    }

    EngineSystemsCreator engine_systems;

    const auto command_texts = rts::Make_Array_View(argv + 1, argc - 1);
    Processor processor;
    Processor::Result result = processor.Parse_Commands(command_texts);

    if (result.id != Processor::ResultId::SUCCESS) {
        captainslog_error("Game Text Compiler failed to parse commands");
        Log_Error(result, command_texts);
        return ProcessorParseError;
    }

    result = processor.Execute_Commands();

    if (result.id != Processor::ResultId::SUCCESS) {
        captainslog_error("Game Text Compiler failed to execute commands");
        Log_Error(result, command_texts);
        return ProcessorExecuteError;
    }

    captainslog_info("Game Text Compiler completed successfully");
    return NoError;
}
