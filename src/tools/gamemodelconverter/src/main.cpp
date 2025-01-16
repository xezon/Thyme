/**
 * @file
 *
 * @author xezon, DevGeniusCode
 *
 * @brief Model Converter. (Thyme Feature)
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

#if defined PLATFORM_WINDOWS
HWND g_applicationHWnd;
#endif

using namespace Thyme;

// clang-format off
void Print_Help()
{
//       1         2         3         4         5         6         7         8         9        10        11        12
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
Print_Line(
R"#(Function Command List ...

Syntax: COMMAND_NAME(ARGUMENT_NAME_A:value,ARGUMENT_NAME_B:value)
All capital words are interpreted keywords and must not be omitted.
All symbols of ( : , ) are part of the syntax and must not be omitted.
'mandatory' and 'optional' words show whether or not argument is mandatory.
[1] and [n] words show that argument takes one or multiple values.
Commands and command arguments are not case sensitive.
Space character will end current Command and begin new Command in command line.
Commands are executed in the order they are written in the command line.

LOAD_W3D(FILE_ID:optional, FILE_PATH:mandatory)
 > Loads a W3D file from FILE_PATH into the FILE_ID slot.
   The W3D file is considered the authoritative format.

LOAD_W3X(FILE_ID:optional, FILE_PATH:mandatory)
 > Loads a W3X file from FILE_PATH into the FILE_ID slot.
   W3X is an XML format representation of W3D.

LOAD_BLEND(FILE_ID:optional, FILE_PATH:mandatory)
 > Loads a Blender (.blend) file from FILE_PATH into the FILE_ID slot.
   Used for non-destructive editing of W3D data.

LOAD_MAX(FILE_ID:optional, FILE_PATH:mandatory)
 > Loads a 3ds Max (.max) file from FILE_PATH into the FILE_ID slot.
   Used for non-destructive editing of W3D data.

SAVE_W3D(FILE_ID:optional, FILE_PATH:mandatory)
 > Saves the W3D file from FILE_ID slot to FILE_PATH.

SAVE_W3X(FILE_ID:optional, FILE_PATH:mandatory)
 > Saves the W3X file from FILE_ID slot to FILE_PATH.

SAVE_BLEND(FILE_ID:optional, FILE_PATH:mandatory)
 > Saves the Blender (.blend) file from FILE_ID slot to FILE_PATH.

SAVE_MAX(FILE_ID:optional, FILE_PATH:mandatory)
 > Saves the 3ds Max (.max) file from FILE_ID slot to FILE_PATH.

RESET(FILE_ID:optional)
 > Resets all loaded data in the specified FILE_ID slot.

SET_OPTIONS(FILE_ID:optional,OPTION:[n]optional)
 > Sets options of OPTION in FILE_ID.
)#");
//       1         2         3         4         5         6         7         8         9        10        11        12
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
Print_Line(
R"#(Command Argument List ...

FILE_ID:number
FILE_ID takes number and allows to manage multiple files in compiler. Default is 0.

FILE_PATH:path
FILE_PATH takes any relative or absolute path.

OPTION:enum
OPTION takes one [1] or multiple [n] options, separated by pipe:
None
)#");
//       1         2         3         4         5         6         7         8         9        10        11        12
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
Print_Line(
R"#(Simplified Command List ...

Commands are executed in the order they are listed here.
All capital words are NOT interpreted keywords and are substituted by the command argument(s) of choice.
[1] and [n] words show that argument takes one or multiple values.
Commands and command arguments are not case sensitive.

WARNING: Commands that save files will overwrite existing files without confirmation.

-load_w3d filepath.w3d
 > Loads a W3D file from the specified file path.

-load_w3x filepath.w3x
 > Loads a W3X file from the specified file path.

-load_blend filepath.blend
 > Loads a Blender file from the specified file path.

-load_max filepath.max
 > Loads a 3ds Max file from the specified file path.

-save_w3d filepath.w3d
 > Saves the loaded data as a W3D file to the given file path.

-save_w3x filepath.w3x
 > Saves the loaded data as a W3X file to the given file path.

-save_blend filepath.blend
 > Saves the loaded data as a Blender file to the given file path.

-save_max filepath.max
 > Saves the loaded data as a 3ds Max file to the given file path.

Example 1: Convert a W3D file to W3X format
 > w3d2w3xcompiler.exe -load_w3d D:\models\model.w3d -convert_w3d_to_w3x D:\models\model.w3x

Example 2: Convert a W3X file to W3D format
 > w3d2w3xcompiler.exe -load_w3x D:\models\model.w3x -convert_w3x_to_w3d D:\models\model.w3d

Example 3: Load a Blender file and save as W3D
 > w3d2w3xcompiler.exe -load_blend D:\models\model.blend -save_w3d D:\models\model.w3d

Example 4: Load a W3D file and save as Max format
 > w3d2w3xcompiler.exe -load_w3d D:\models\model.w3d -save_max D:\models\model.max
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

struct CaptainsLogCreator
{
    CaptainsLogCreator()
    {
        captains_settings_t captains_settings = { 0 };
        captains_settings.level = LOGLEVEL_DEBUG;
        captains_settings.console = true;
        captains_settings.print_file = true;
        captainslog_init(&captains_settings);
    };
    ~CaptainsLogCreator() { captainslog_deinit(); }
};

struct EngineSystemsCreator
{
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
    Print_Line("W3D W3X Compiler v1.0 By The Assembly Armada");

    if (argc < 2) {
        Print_Help();
        return MissingArgumentsError;
    }
    // TODO: W3DFILE
    GameModelFILE::Set_Log_File(stderr);
    CaptainsLogCreator captains_log_creator;
    EngineSystemsCreator engine_systems_creator;

    const auto command_texts = rts::Make_Array_View(argv + 1, argc - 1);
    Processor processor;
    Processor::Result result = processor.Parse_Commands(command_texts);

    if (result.id != Processor::ResultId::SUCCESS) {
        Print_Line("ERROR : Model Converter failed to parse commands");
        Print_Error(result, command_texts);
        return ProcessorParseError;
    }

    result = processor.Execute_Commands();

    if (result.id != Processor::ResultId::SUCCESS) {
        Print_Line("ERROR : Model Converter failed to execute commands");
        Print_Error(result, command_texts);
        return ProcessorExecuteError;
    }

    Print_Line("Model Converter completed successfully");
    return NoError;
}
