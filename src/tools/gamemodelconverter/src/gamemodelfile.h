/**
 * @file gamemodelfile.h
 *
 * @author DevGeniusCode
 *
 * @brief Game Model File. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "utility/ebitflags.h"
#include "chunksmanager.h"
#include "filesystem.h"
#include <string>


namespace Thyme
{

enum class GameModelOption : uint32_t
{
    NONE = 0,
};

bool Name_To_Game_Model_Option(const char *name, GameModelOption &option);

} // namespace Thyme

DEFINE_RTS_UNDERLYING_TYPE(Thyme::GameModelOption, uint32_t);

namespace Thyme
{

// #TODO Return error codes for load and save?

class GameModelFile
{
public:
    using Options = rts::ebitflags<GameModelOption>;

public:
    GameModelFile();
    ~GameModelFile(){
        Reset();
    }

    // TODO Checks whether chunks is loaded.
    bool Is_Loaded() const;

    // Loads models file from disk. Does not unload previous data on failure.
    bool Load(const char *filename);
    bool Load_W3D(const char *filename);
    bool Load_W3X(const char *filename);
    bool Load_BLEND(const char *filename);
    bool Load_MAX(const char *filename);

    // Saves models file to disk. Will write over any existing file.
    bool Save(const char *filename);
    bool Save_W3D(const char *filename);
    bool Save_W3X(const char *filename);
    bool Save_BLEND(const char *filename);
    bool Save_MAX(const char *filename);

    // Unloads all data and resets all settings.
    void Reset();

    // Sets options for loading and saving files.
    void Set_Options(Options options);
    Options Get_Options() const;

    // Optional logging stream. Works besides captains log.
    static void Set_Log_File(FILE *log);

private:
    enum class FileType
    {
        AUTO,
        W3D,
        W3X,
        BLEND,
        MAX,

        COUNT,
    };


private:
    bool Load(const char *filename, FileType filetype);
    bool Save(const char *filename, FileType filetype);

    static FileType Get_File_Type(const char *filename, FileType filetype);

    bool Read_W3D_File(const char *filename, const Options& options);
    bool Write_W3D_File(const char *filename, const Options& options);

    static void Log_Line(const char *prefix, const char *format, ...);

private:
    Options m_options;
    static FILE *s_logfile;
    ChunkTreePtr m_rootChunk;
    ChunkManager * m_chunkManager;
    ChunkLoadClass m_chunkLoader;
};

} // namespace Thyme
