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
        size_t totalLength;
        size_t chunkCount;

        LengthInfo() : totalLength(0), chunkCount(0) {}

        void AddChunk(size_t length) {
            totalLength += length;
            chunkCount++;
        }

        void Reset() {
            totalLength = 0;
            chunkCount = 0;
        }
    };

    // TODO chunk size
    enum : size_t
    {
        CHUNK_SIZE = 1024
    };

    using ChunkInfosPtrArray = std::vector<std::shared_ptr<ChunkInfos>>;
    using ConstChunkInfosPtrArray = rts::array<const ChunkInfos *, CHUNK_SIZE>;
    using ChunkInfosArray = rts::array<ChunkInfos, CHUNK_SIZE>;
    using Utf8Array = rts::array<char, CHUNK_SIZE>;
    using Utf16Array = rts::array<unichar_t, CHUNK_SIZE>;
    using Utf8View = rts::array_view<char>;
    using Utf16View = rts::array_view<unichar_t>;

private:
    bool Load(const char *filename, FileType filetype);
    bool Save(const char *filename, FileType filetype);

    void Check_Buffer_Lengths();

    ChunkInfos &Mutable_Chunk_Infos();

    static ChunkInfosPtrArray Build_Chunk_Infos_Ptrs_Array(ChunkInfosArray &chunk_infos_array);
    static ConstChunkInfosPtrArray Build_Const_Chunk_Infos_Ptrs_Array(
        ChunkInfosArray &chunk_infos_array);

    static size_t Get_Max_Size(const ConstChunkInfosPtrArray &chunk_infos_ptrs);

    static FileType Get_File_Type(const char *filename, FileType filetype);

    static void Collect_Length_Info(LengthInfo &len_info, const ChunkInfos &chunks);
    static void Log_Length_Info(const LengthInfo &len_info);
    static void Assert_Length_Info(const LengthInfo &len_info);

	// TODO Get_Specific_Chunk
    static ChunkInfos &Get_Specific_Chunk(ChunkInfo &chunk_info);

    static bool Read_W3D_File(FileRef &file, ChunkInfos &chunk_infos, Options options);
    template<typename ChunkInfosType>
    static void Read_W3D_File_T(FileRef &file, ChunkInfosType &chunk_infos, Options options);
	// TODO Parse_W3D_Specific_Step
    static ModelParseResult Parse_W3D_Specific_Step(Utf8Array &buf, ChunkInfo &chunk, Options options);
    static bool Is_W3D_Pre_Specific(Utf8View buf);
    static bool Is_W3D_Comment(const char *cchunk);
    static bool Is_W3D_End(const char *cchunk);
    static void Change_Step(ModelReadStep &step, ModelReadStep new_step, const char *&eol_chars);

    // TODO read specific chunk
    static bool Read_W3D_Header(FileRef &file, ChunkInfos &chunk_infos);
    static bool Read_W3D_Entry(FileRef &file, ChunkInfo &chunk_info, Options options, Utf16Array &buf);

    static bool Write_W3D_File(FileRef &file, const ChunkInfos &chunk_infos, Options options);
    static bool Write_W3D_Entry(
        FileRef &file, const ChunkInfo &chunk_info, Options options, Utf8Array &buf, ChunkInfo &str);
    static bool Write_W3D_Chunk(FileRef &file, const ChunkInfo &chunk);
    static bool Write_W3D_End(FileRef &file);

    static void Log_Line(const char *prefix, const char *format, ...);

private:
    Options m_options;
    ChunkInfosArray m_chunkInfosArray;
    ChunkInfos m_chunkInfos;
    static FILE *s_logfile;
};

} // namespace Thyme
