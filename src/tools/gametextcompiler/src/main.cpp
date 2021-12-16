/**
 * @file
 *
 * @author xezon
 *
 * @brief Game Text Compiler
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

void Log_Help()
{
    // clang-format off
    captainslog_info(
        "Command List:\n"
        "\n"
        "No 01 : LOAD(FILE_ID:optional,FILE_PATH:mandatory)\n"
        "No 02 : LOAD_CSF(FILE_ID:optional,FILE_PATH:mandatory)\n"
        "No 03 : LOAD_STR(FILE_ID:optional,FILE_PATH:mandatory,LANGUAGE:[n]optional)\n"
        "No 04 : SAVE(FILE_ID:optional,FILE_PATH:mandatory)\n"
        "No 05 : SAVE_CSF(FILE_ID:optional,FILE_PATH:mandatory,LANGUAGE:[1]optional)\n"
        "No 06 : SAVE_STR(FILE_ID:optional,FILE_PATH:mandatory,LANGUAGE:[n]optional)\n"
        "No 07 : UNLOAD(FILE_ID:optional,LANGUAGE:[n]optional)\n"
        "No 08 : RESET(FILE_ID:optional)\n"
        "No 09 : MERGE_AND_OVERWRITE(FILE_ID:mandatory,FILE_ID:mandatory,LANGUAGE:[n]optional)\n"
        "No 10 : SET_OPTIONS(FILE_ID:optional,OPTION:[n]optional)\n"
        "No 11 : SET_LANGUAGE(FILE_ID:optional,LANGUAGE:[1]mandatory)\n"
        "No 12 : SWAP_LANGUAGE_STRINGS(FILE_ID:optional,LANGUAGE:[1]mandatory,LANGUAGE:[1]mandatory)\n"
        "\n"
        ".. 01 : Loads a STR or CSF file from FILE_PATH into FILE_ID slot.\n"
        ".. 02 : Loads a CSF file from FILE_PATH into FILE_ID slot.\n"
        ".. 03 : Loads a STR file from FILE_PATH with LANGUAGE into FILE_ID slot.\n"
        ".. 04 : Saves a STR or CSF file to FILE_PATH from FILE_ID slot.\n"
        ".. 05 : Saves a CSF file to FILE_PATH from FILE_ID slot.\n"
        ".. 06 : Saves a STR file to FILE_PATH with LANGUAGE into FILE_ID slot.\n"
        ".. 07 : Unloads string data of LANGUAGE from FILE_ID slot.\n"
        ".. 08 : Resets all string data.\n"
        ".. 09 : Merges and overwrites string data of LANGUAGE in 1st FILE_ID from 2nd FILE_ID.\n"
        ".. 10 : Sets options of OPTION in FILE_ID.\n"
        ".. 11 : Sets language of LANGUAGE in FILE_ID.\n"
        ".. 12 : Swaps string data in FILE_ID between 1st LANGUAGE and 2nd LANGUAGE.\n"
    );
    captainslog_info(
        "Command Argument List:\n"
        "\n"
        "No 01 : FILE_ID:Number\n"
        "No 02 : FILE_PATH:Path\n"
        "No 03 : LANGUAGE:English|German|French|Spanish|Italian|Korean|Chinese|Brazilian|Polish|Russian|Arabic\n"
        "No 04 : OPTION:None|Check_Buffer_Length_On_Load|Check_Buffer_Length_On_Save|\n"
        "               Keep_Spaces_On_STR_Load|Print_Linebreaks_On_STR_Save|Optimize_Memory_Size\n"
        "\n"
        ".. 01 : FILE_ID takes number and allows to manage multiple files in compiler. Default is 0.\n"
        ".. 02 : FILE_PATH takes any relative or absolute path.\n"
        ".. 03 : LANGUAGE takes one [1] or multiple [n] languages.\n"
        ".. 04 : OPTION takes one [1] or multiple [n] options.\n"
    );
    // clang-format on
}

namespace
{
constexpr int NoError = 0;
constexpr int MissingArgumentsError = 1;
constexpr int ProcessorPrepareError = 2;
constexpr int ProcessorRunError = 3;
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
    }
};

// #TODO Add alternative command line argument format if so required for user.

int main(int argc, const char *argv[])
{
    CaptainsLogCreator captains_log;

    captainslog_info("Thyme Game Text Compiler 1.0");

    if (argc < 2) {
        Log_Help();
        return MissingArgumentsError;
    }

    EngineSystemsCreator engine_systems;

    Thyme::Processor::Result processor_result;
    Thyme::Processor processor;

    const auto commands = rts::Make_Array_View(argv + 1, argc - 1);
    processor_result = processor.Prepare(commands);

    if (processor_result != Thyme::Processor::Result::SUCCESS) {
        captainslog_info("Processor prepare failed");
        return ProcessorPrepareError;
    }

    processor_result = processor_result = processor.Execute_Commands();

    if (processor_result != Thyme::Processor::Result::SUCCESS) {
        captainslog_info("Processor run failed");
        return ProcessorRunError;
    }

    captainslog_info("Completed successfully");
    return NoError;
}
