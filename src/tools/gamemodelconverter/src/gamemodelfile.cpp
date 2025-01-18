/**
 * @file gamemodelfile.cpp
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
#include "always.h"
#include "gamemodelfile.h"
#include "filesystem.h"
#include "rtsutils.h"
#include "utility/arrayutil.h"
#include "utility/fileutil.h"
#include "utility/stlutil.h"
#include "utility/stringutil.h"
#include "w3dfilesystem.h"

#define GAMEMODELLOG_TRACE(fmt, ...) \
    captainslog_trace(fmt, ##__VA_ARGS__); \
    GameModelFile::Log_Line("TRACE : ", fmt, ##__VA_ARGS__)

#define GAMEMODELLOG_DEBUG(fmt, ...) \
    captainslog_debug(fmt, ##__VA_ARGS__); \
    GameModelFile::Log_Line("DEBUG : ", fmt, ##__VA_ARGS__)

#define GAMEMODELLOG_INFO(fmt, ...) \
    captainslog_info(fmt, ##__VA_ARGS__); \
    GameModelFile::Log_Line("", fmt, ##__VA_ARGS__)

#define GAMEMODELLOG_WARN(fmt, ...) \
    captainslog_warn(fmt, ##__VA_ARGS__); \
    GameModelFile::Log_Line("WARNING : ", fmt, ##__VA_ARGS__)

#define GAMEMODELLOG_ERROR(fmt, ...) \
    captainslog_error(fmt, ##__VA_ARGS__); \
    GameModelFile::Log_Line("ERROR : ", fmt, ##__VA_ARGS__)

#define GAMEMODELLOG_FATAL(fmt, ...) \
    captainslog_fatal(fmt, ##__VA_ARGS__); \
    GameModelFile::Log_Line("FATAL : ", fmt, ##__VA_ARGS__)

namespace Thyme
{

namespace
{
template<typename IntegerType> constexpr size_t Bit_To_Index(IntegerType integer)
{
    using UnsignedInt = UnsignedIntegerT<IntegerType>;
    size_t n = 0;
    for (; n < sizeof(IntegerType) * 8; ++n) {
        if (UnsignedInt(integer) & (UnsignedInt(1) << n)) {
            return n;
        }
    }
    return n;
}

constexpr const char *const s_option_0 = "None";

constexpr const char *const s_options[] = {
    s_option_0,
};

static_assert(s_option_0 == s_options[size_t(GameModelOption::NONE)]);
} // namespace

bool Name_To_Game_Model_Option(const char *name, GameModelOption &option)
{
    size_t index = 0;
    for (const char *option_name : s_options) {
        if (strcasecmp(option_name, name) == 0) {
            option = (index == 0) ? GameModelOption::NONE : static_cast<GameModelOption>(1 << (index - 1));
            return true;
        }
        ++index;
    }
    return false;
}

FILE *GameModelFile::s_logfile = nullptr;
GameModelFile::GameModelFile() : // Initialize with default options
    m_options(GameModelOption::NONE), m_chunkInfosArray() {};

bool GameModelFile::Is_Loaded() const
{
    return !Get_Chunk_Infos().empty();
}

bool GameModelFile::Load(const char *filename)
{
    const FileType filetype = Get_File_Type(filename, FileType::AUTO);

    return Load(filename, filetype);
}

bool GameModelFile::Load_W3D(const char *filename)
{
    return Load(filename, FileType::W3D);
}

bool GameModelFile::Load_W3X(const char *filename)
{
    return Load(filename, FileType::W3X);
}

bool GameModelFile::Load_BLEND(const char *filename)
{
    return Load(filename, FileType::BLEND);
}

bool GameModelFile::Load_MAX(const char *filename)
{
    return Load(filename, FileType::MAX);
}

bool GameModelFile::Save(const char *filename)
{
    FileType filetype = Get_File_Type(filename, FileType::AUTO);

    return Save(filename, filetype);
}

bool GameModelFile::Save_W3D(const char *filename)
{
    return Save(filename, FileType::W3D);
}

bool GameModelFile::Save_W3X(const char *filename)
{
    return Save(filename, FileType::W3X);
}

bool GameModelFile::Save_BLEND(const char *filename)
{
    return Save(filename, FileType::BLEND);
}

bool GameModelFile::Save_MAX(const char *filename)
{
    return Save(filename, FileType::MAX);
}

const ChunkInfos &GameModelFile::Get_Chunk_Infos() const
{
    // TODO size/vector of chunks
    return m_chunkInfosArray[size_t(CHUNK_SIZE)];
}

void GameModelFile::Set_Options(Options options)
{
    m_options = options;
}

GameModelFile::Options GameModelFile::Get_Options() const
{
    return m_options;
}

void GameModelFile::Set_Log_File(FILE *log)
{
    s_logfile = log;
}

// Private Methods

bool GameModelFile::Load(const char *filename, FileType filetype)
{
    captainslog_assert(filetype != FileType::AUTO);

    if (!filename || !filename[0]) {
        GAMEMODELLOG_ERROR("File without name cannot be loaded");
        return false;
    }

    const int filemode = Encode_Buffered_File_Mode(File::READ | File::BINARY, 1024 * 32);
    // TODO: Check if this is the correct way to open a file, there is g_theW3DFileSystem
    FileRef file = g_theFileSystem->Open_File(filename, filemode);

    if (!file.Is_Open()) {
        GAMEMODELLOG_ERROR("File '%s' cannot be opened for read", filename);
        return false;
    }

    bool success = false;

    ChunkInfosArray chunk_infos_array;
    // TODO size of chunks
    ChunkInfos &chunk_infos = chunk_infos_array[size_t(CHUNK_SIZE)];

    switch (filetype) {

        case FileType::W3D: {
            success = Read_W3D_File(file, chunk_infos, m_options);
            // TODO size of chunks
            //            chunk_infos_array[size_t(CHUNK_SIZE)].swap(chunk_infos);
            break;
        }
        case FileType::W3X: {
            // TODO: W3X
            //            success = Read_W3X_File(file, chunk_infos, m_options);
            break;
        }
    }

    if (success) {
        GAMEMODELLOG_INFO("File '%s' loaded successfully", filename);
    } else {
        GAMEMODELLOG_ERROR("File '%s' failed to load", filename);
    }

    return success;
}

bool GameModelFile::Save(const char *filename, FileType filetype)
{
    captainslog_assert(filetype != FileType::AUTO);

    if (!filename || !filename[0]) {
        GAMEMODELLOG_ERROR("File without name cannot be saved");
        return false;
    }

    // TODO Is_Any_Loaded for chunks
    //    if (!Is_Any_Loaded()) {
    //        GAMEMODELLOG_ERROR("File without chunk data cannot be saved");
    //        return false;
    //    }

    const int filemode = Encode_Buffered_File_Mode(File::WRITE | File::CREATE | File::BINARY, 1024 * 32);
    FileRef file = g_theFileSystem->Open_File(filename, filemode);

    if (!file.Is_Open()) {
        GAMEMODELLOG_ERROR("File '%s' cannot be opened for write", filename);
        return false;
    }

    bool success = false;

    switch (filetype) {

        case FileType::W3D: {
            success = Write_W3D_File(file, Get_Chunk_Infos(), m_options);
            break;
        }
        case FileType::W3X: {
            // TODO: W3X
            //            success = Write_W3X_File(file, Get_Chunk_Infos(), m_options);
            break;
        }
    }

    if (success) {
        GAMEMODELLOG_INFO("File '%s' saved successfully", filename);

    } else {
        GAMEMODELLOG_ERROR("File '%s' failed to save", filename);
    }

    return success;
}

void GameModelFile::Reset()
{
    for (ChunkInfos &chunk_infos : m_chunkInfosArray) {
        // TODO size/vector of chunks
        rts::Free_Container(chunk_infos);
    }
    m_options = Options::Value::NONE;
}

void GameModelFile::Check_Buffer_Lengths()
{
    // TODO: Implement
}

ChunkInfos &GameModelFile::Mutable_Chunk_Infos()
{
    // TODO size/vector of chunks
    return m_chunkInfosArray[size_t(CHUNK_SIZE)];
}

ChunkInfosPtrArray GameModelFile::Build_Chunk_Infos_Ptrs_Array(ChunkInfosArray &chunk_infos_array)
{
    // TODO: Implement
    return {};
}

ConstChunkInfosPtrArray GameModelFile::Build_Const_Chunk_Infos_Ptrs_Array(ChunkInfosArray &chunk_infos_array)
{
    // TODO: Implement
    return {};
}

size_t GameModelFile::Get_Max_Size(const ConstChunkInfosPtrArray &chunk_infos_ptrs)
{
    // TODO: Implement
    return 0;
}

FileType GameModelFile::Get_File_Type(const char *filename, FileType filetype)
{
    // TODO: Implement
    return FileType::AUTO;
}

void GameModelFile::Collect_Length_Info(LengthInfo &len_info, const ChunkInfos &chunks)
{
    // TODO: Implement
}

void GameModelFile::Log_Length_Info(const LengthInfo &len_info)
{
    // TODO: Implement
}

void GameModelFile::Assert_Length_Info(const LengthInfo &len_info)
{
    // TODO: Implement
}

ChunkInfos &GameModelFile::Get_Specific_Chunk(ChunkInfo &chunk_info)
{
    // TODO: Implement
    return m_chunkInfos;
}

bool GameModelFile::Read_W3D_File(FileRef &file, ChunkInfos &chunk_infos, Options options)
{
    // TODO: Implement
    return false;
}

template<typename ChunkInfosType>
void GameModelFile::Read_W3D_File_T(FileRef &file, ChunkInfosType &chunk_infos, Options options)
{
    // TODO: Implement
}

ModelParseResult GameModelFile::Parse_W3D_Specific_Step(Utf8Array &buf, ChunkInfo &chunk, Options options)
{
    // TODO: Implement
    return ModelParseResult::IS_NOTHING;
}

bool GameModelFile::Is_W3D_Pre_Specific(Utf8View buf)
{
    // TODO: Implement
    return false;
}

bool GameModelFile::Is_W3D_Comment(const char *cchunk)
{
    // TODO: Implement
    return false;
}

bool GameModelFile::Is_W3D_End(const char *cchunk)
{
    // TODO: Implement
    return false;
}

void GameModelFile::Change_Step(ModelReadStep &step, ModelReadStep new_step, const char *&eol_chars)
{
    // TODO: Implement
}

bool GameModelFile::Read_W3D_Header(FileRef &file, ChunkInfos &chunk_infos)
{
    // TODO: Implement
    return false;
}

bool GameModelFile::Read_W3D_Entry(FileRef &file, ChunkInfo &chunk_info, Options options, Utf16Array &buf)
{
    // TODO: Implement
    return false;
}

bool GameModelFile::Write_W3D_File(FileRef &file, const ChunkInfos &chunk_infos, Options options)
{
    // TODO: Implement
    return false;
}

bool GameModelFile::Write_W3D_Entry(
    FileRef &file, const ChunkInfo &chunk_info, Options options, Utf8Array &buf, ChunkInfo &str)
{
    // TODO: Implement
    return false;
}

bool GameModelFile::Write_W3D_Chunk(FileRef &file, const ChunkInfo &chunk)
{
    // TODO: Implement
    return false;
}

bool GameModelFile::Write_W3D_End(FileRef &file)
{
    // TODO: Implement
    return false;
}

void GameModelFile::Log_Line(const char *prefix, const char *format, ...)
{
    // TODO: Implement
}

} // namespace Thyme

#undef GAMEMODELLOG_TRACE
#undef GAMEMODELLOG_DEBUG
#undef GAMEMODELLOG_INFO
#undef GAMEMODELLOG_WARN
#undef GAMEMODELLOG_ERROR
#undef GAMEMODELLOG_FATAL
