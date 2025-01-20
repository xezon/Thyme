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

#include "fileref.h"
#include "gamemodelcommon.h"
#include "utility/array.h"
#include "utility/arrayview.h"
#include "utility/ebitflags.h"
#include "utility/enumerator.h"
#include "utility/enumflags.h"

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

    // Checks whether model is loaded.
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

    // Retrieves all Chunk data.
    const ChunkInfos &Get_Chunk_Infos() const;

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

    enum class ModelReadStep
    {
        // TODO ModelReadStep
        INITIAL,
        HEADER,
        CHUNK,
        COMPLETE,
    };

    enum class ModelParseResult
    {
        // TODO ModelParseResult
        IS_HEADER,
        IS_CHUNK,
        IS_NOTHING,
        IS_END,
    };

    struct LengthInfo
    {
        // TODO LengthInfo of chunks
        int max_chunk_len;
    };


private:
    bool Load(const char *filename, FileType filetype);
    bool Save(const char *filename, FileType filetype);

//    void Check_Buffer_Lengths();
//
//    ChunkInfos &Mutable_Chunk_Infos();
//
//    static ChunkInfosPtrArray Build_Chunk_Infos_Ptrs_Array(ChunkInfosArray &chunk_infos_array);
//    static ConstChunkInfosPtrArray Build_Const_Chunk_Infos_Ptrs_Array(
//        ChunkInfosArray &chunk_infos_array);

    static size_t Get_Max_Size(const ConstChunkInfosPtrArray &chunk_infos_ptrs);

    static FileType Get_File_Type(const char *filename, FileType filetype);

//    static void Collect_Length_Info(LengthInfo &len_info, const ChunkInfos &chunks);
//    static void Log_Length_Info(const LengthInfo &len_info);
//    static void Assert_Length_Info(const LengthInfo &len_info);

    // TODO need a significant rewrite based on the chunk structure
    static Utf8Array &Get_Chunk_Data(ChunkInfo &chunk_info);

    bool Read_W3D_File(FileRef &file, ChunkInfos &chunk_infos, Options options);

    // TODO need substantial changes based on the chunk structure
    // Recursive function to read/write chunks and subchunks
    bool Read_W3D_Chunks(FileRef &file, ChunkInfos& parentChunks);
    bool Write_W3D_Chunks(FileRef &file, const ChunkInfos& parentChunks);

    //Helper function to write a single chunk to the file
    bool Write_W3D_Chunk(FileRef &file, const ChunkInfo& chunk);

    bool Write_W3D_File(FileRef &file, const ChunkInfos &chunk_infos, Options options);

    void Parse_W3D_Data(const ChunkInfos& chunks);
    void Parse_Mesh_Chunk(const ChunkInfo& meshChunk);
    void Parse_Emitter_Chunk(const ChunkInfo& emitterChunk);
    const ParsedMeshData &GetMeshData() const;
    const ParsedEmitterData &GetEmitterData() const;

    static void Log_Line(const char *prefix, const char *format, ...);

private:
    Options m_options;
    ChunkInfos m_chunkInfos;
    // TODO chunks array/buffer
    static FILE *s_logfile;
    ParsedMeshData m_meshData;
    ParsedEmitterData m_emitterData;

};

} // namespace Thyme
