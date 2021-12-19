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
R"#(Function Command List (All capital words are interpreted keywords):

Fu 01 : LOAD_CSF(FILE_ID:optional,FILE_PATH:mandatory)
Fu 02 : LOAD_STR(FILE_ID:optional,FILE_PATH:mandatory,LANGUAGE:[n]optional)
Fu 03 : SAVE_CSF(FILE_ID:optional,FILE_PATH:mandatory)
Fu 04 : SAVE_STR(FILE_ID:optional,FILE_PATH:mandatory,LANGUAGE:[n]optional)
Fu 05 : UNLOAD(FILE_ID:optional,LANGUAGE:[n]optional)
Fu 06 : RESET(FILE_ID:optional)
Fu 07 : MERGE_AND_OVERWRITE(FILE_ID:mandatory,FILE_ID:mandatory,LANGUAGE:[n]optional)
Fu 08 : SET_OPTIONS(FILE_ID:optional,OPTION:[n]optional)
Fu 09 : SET_LANGUAGE(FILE_ID:optional,LANGUAGE:[1]mandatory)
Fu 10 : SWAP_LANGUAGE_STRINGS(FILE_ID:optional,LANGUAGE:[1]mandatory,LANGUAGE:[1]mandatory)

.. 01 : Loads a CSF file from FILE_PATH into FILE_ID slot.
.. 02 : Loads a STR file from FILE_PATH with LANGUAGE into FILE_ID slot.
.. 03 : Saves a CSF file to FILE_PATH from FILE_ID slot.
.. 04 : Saves a STR file to FILE_PATH with LANGUAGE into FILE_ID slot.
.. 05 : Unloads string data of LANGUAGE from FILE_ID slot.
.. 06 : Resets all string data.
.. 07 : Merges and overwrites string data of LANGUAGE in 1st FILE_ID from 2nd FILE_ID.
.. 08 : Sets options of OPTION in FILE_ID.
.. 09 : Sets language of LANGUAGE in FILE_ID.
.. 10 : Swaps string data in FILE_ID between 1st LANGUAGE and 2nd LANGUAGE.
)#");

captainslog_info(
R"#(Command Argument List:

Ar 01 : FILE_ID:Number
Ar 02 : FILE_PATH:Path
Ar 03 : LANGUAGE:English|German|French|Spanish|Italian|Japanese|Korean|Chinese|Brazilian|Polish|Unknown|Russian|Arabic
Ar 04 : OPTION:None|Check_Buffer_Length_On_Load|Check_Buffer_Length_On_Save|Keep_Spaces_On_STR_Load|Print_Linebreaks_On_STR_Save|Optimize_Memory_Size

.. 01 : FILE_ID takes number and allows to manage multiple files in compiler. Default is 0.
.. 02 : FILE_PATH takes any relative or absolute path.
.. 03 : LANGUAGE takes one [1] or multiple [n] languages.
.. 04 : OPTION takes one [1] or multiple [n] options.
)#");

captainslog_info(
R"#(Simplified Command List (All capital words are interpreted keywords):

Si 01 : -OPTIONS        option[n]
Si 02 : -LOAD_LANGUAGES language[n]
Si 03 : -LOAD_CSF_FILE  filepath.csf
Si 04 : -LOAD_STR_FILE  filepath.str
Si 05 : -SAVE_LANGUAGES language[n]
Si 06 : -SAVE_CSF       filepath.csf
Si 07 : -SAVE_STR       filepath.str

.. 01 : Sets option(s) for loaded and saved file (see Command Argument List).
.. 02 : Sets language(s) to load file with (see Command Argument List).
.. 03 : Loads a CSF file from given file path. If LOAD_LANGUAGES is specified, then CSF loads in first language as specified by LOAD_LANGUAGES, else CSF loads in language as stored by CSF file.
.. 04 : Loads a STR file from given file path. If LOAD_LANGUAGES is specified, then STR loads in multi language format, else STR loads in regular format with unknown language.
.. 05 : Sets language(s) to save file with (see Command Argument List).
.. 06 : Saves a CSF file to given file path. If SAVE_LANGUAGES is specified, then CSF saves in first language as specified by LOAD_LANGUAGES, else CSF saves in language as was loaded with.
.. 07 : Saves a STR file to given file path. If SAVE_LANGUAGES is specified, then STR saves in multi language format, else STR saves in regular format with unknown language.
)#");

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
    processor_result = processor.Parse_Commands(commands);

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
