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
#include "gamemodelcommon.h"
#include "w3d_file.h"


#define GAMEMODELLOG_TRACE(fmt, ...) \
    captainslog_trace(fmt, ##__VA_ARGS__); \
    GameModelFile::Log_Line("TRACE : ", fmt, ##__VA_ARGS__)

#define GAMEMODELLOG_DEBUG(fmt, ...) \
    captainslog_debug(fmt, ##__VA_ARGS__); \
    GameModelFile::Log_Line("DEBUG : ", fmt, ##__VA_ARGS__)

#define GAMEMODELLOG_INFO(fmt, ...) \
    captainslog_info(fmt, ##__VA_ARGS__); \
    GameModelFile::Log_Line("INFO : ", fmt, ##__VA_ARGS__)

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
    m_options(GameModelOption::NONE), m_chunkInfos() {};

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
    return m_chunkInfos;
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
    Reset();

    captainslog_assert(filetype != FileType::AUTO);

    if (!filename || !filename[0]) {
        GAMEMODELLOG_ERROR("File without name cannot be loaded");
        return false;
    }


    // TODO: Check if this is the correct way to open a file, there is g_theW3DFileSystem, but he use game path, not the file path
    const int filemode = Encode_Buffered_File_Mode(File::READ | File::BINARY, 1024 * 32);
    FileRef file = g_theFileSystem->Open_File(filename, filemode);
    if (!file.Is_Open()) {
        GAMEMODELLOG_ERROR("File '%s' cannot be opened for read", filename);
        return false;
    }

    bool success = false;

    switch (filetype) {

        case FileType::W3D: {
            success = Read_W3D_File(file, m_chunkInfos, m_options);
            break;
        }
        case FileType::W3X:
            // success = Read_W3X_File(file, chunk_infos, m_options);
        case FileType::BLEND:
        case FileType::MAX: {
            GAMEMODELLOG_WARN("Loading for file type '%s' not yet implemented.", filetype);
            break;
        }
        default:
            GAMEMODELLOG_ERROR("Unknown file type: %d", filetype);
            break;
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
        case FileType::W3X:
        case FileType::BLEND:
        case FileType::MAX: {
            // success = Write_W3X_File(file, Get_Chunk_Infos(), m_options);
            GAMEMODELLOG_WARN("Saving for file type '%s' not yet implemented.", filetype);
            success = false;
            break;
        }
        default:
            GAMEMODELLOG_ERROR("Unknown file type: %d", filetype);
            success = false;
            break;
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
    // TODO size/vector of chunks
    //    for (ChunkInfos &chunk_infos : m_chunkInfosArray) {
    //        rts::Free_Container(chunk_infos);
    //    }
    m_chunkInfos.clear();
    m_options = Options::Value::NONE;
}

// void GameModelFile::Check_Buffer_Lengths()
//{
//     // TODO: Implement buffer management
// }
//
// ChunkInfos &GameModelFile::Mutable_Chunk_Infos()
//{
//     // TODO size/vector of chunks
//     return m_chunkInfosArray[size_t(CHUNK_SIZE)];
// }
//
// ChunkInfosPtrArray GameModelFile::Build_Chunk_Infos_Ptrs_Array(ChunkInfosArray &chunk_infos_array)
//{
//     // TODO: Implement
//     return {};
// }
//
// ConstChunkInfosPtrArray GameModelFile::Build_Const_Chunk_Infos_Ptrs_Array(ChunkInfosArray &chunk_infos_array)
//{
//     // TODO: Implement
//     return {};
// }
//
// size_t GameModelFile::Get_Max_Size(const ConstChunkInfosPtrArray &chunk_infos_ptrs)
//{
//     // TODO size/vector of chunks
//     return 0;
// }

GameModelFile::FileType GameModelFile::Get_File_Type(const char *filename, FileType filetype)
{
    if (filetype == FileType::AUTO) {
        std::string fn(filename);
        if (fn.find(".w3d") != std::string::npos)
            return FileType::W3D;
        // Add checks for .w3x, .blend, .max here
        if (fn.find(".w3x") != std::string::npos)
            return FileType::W3X;
        if (fn.find(".blend") != std::string::npos)
            return FileType::BLEND;
        if (fn.find(".max") != std::string::npos)
            return FileType::MAX;
    }
    return filetype;
}

bool GameModelFile::Read_W3D_File(FileRef &file, ChunkInfos &chunk_infos, Options options)
{
    GAMEMODELLOG_INFO("Reading model file '%s' in W3D format", file.Get_File_Name().Str());

    if (!file.Is_Open()) {
        GAMEMODELLOG_ERROR("Could not open file: %s\n", file.Get_File_Name().Str());
        return false;
    }

    if (!Read_W3D_Chunks(file, chunk_infos)) {
        GAMEMODELLOG_ERROR("Failed to read W3D chunks from: %s\n", file.Get_File_Name().Str());
        return false;
    }

    //Parse the loaded chunks here (example for mesh data)
    Parse_Model_Data(chunk_infos);
    return true;
}

bool GameModelFile::Write_W3D_File(FileRef &file, const ChunkInfos &chunk_infos, Options options)
{
    GAMEMODELLOG_INFO("Writing model file '%s' in W3D format", file.Get_File_Name().Str());

    bool success = true;
    if (!file.Is_Open()) { // Use FM_WRITE for saving
        GAMEMODELLOG_ERROR("File '%s' cannot be opened for write", file.Get_File_Name().Str());
        success = false;
    }
    if (!Write_W3D_Chunks(file, chunk_infos)) {
        GAMEMODELLOG_ERROR("Failed to write W3D chunks to: %s\n", file.Get_File_Name().Str());
        success = false;
    }

    return success;
}

bool GameModelFile::Read_W3D_Chunks(FileRef &file, ChunkInfos &parentChunks)
{
    while (true) {
        uint32_t chunkType;
        uint32_t chunkSizeRaw;

        // Read chunk header (type + size) together
        if (file.Read(&chunkType, sizeof(chunkType)) != sizeof(chunkType) ||
            file.Read(&chunkSizeRaw, sizeof(chunkSizeRaw)) != sizeof(chunkSizeRaw)) {
            break; // End of file or error
        }

        // Prepare the chunk information
        ChunkInfo chunk;
        chunk.chunkType = chunkType;
        size_t dataSize = chunkSizeRaw & 0x7FFFFFFF; // Mask out MSB for subChunk flag
        chunk.chunkSize = dataSize;
        bool hasSubchunks = (chunkSizeRaw >> 31) & 1; // Check MSB for subchunks (chunkSizeRaw & 0x80000000)

        int currentPosition = file.Position(); // Save current file position

        if (hasSubchunks) {
            // Read subchunks recursively
            while (file.Position() < currentPosition + dataSize) {
                if (!Read_W3D_Chunks(file, chunk.subChunks)) {
                    GAMEMODELLOG_ERROR("Failed to read subchunks for chunk type 0x%X\n", chunkType);
                    return false;
                }
            }
        } else {
            // Read the current chunk data
            chunk.data.resize(dataSize);
            if (file.Read(chunk.data.data(), dataSize) != dataSize) {
                GAMEMODELLOG_ERROR("File '%s': Failed to read chunk data (type: 0x%X).\n", file.Get_File_Name().Str(), chunkType);
                return false;
            }
        }

        // Check if we are at the correct position after reading chunk data
        if (file.Position() != currentPosition + dataSize) {
            GAMEMODELLOG_ERROR("Incorrect position after reading chunk data (type: 0x%X).\n", chunkType);
        }

        // Add chunk to the parent chunk list
        parentChunks.push_back(chunk);

        GAMEMODELLOG_DEBUG("Read chunk: type=0x%X, size=%zu, subchunks=%d", chunkType, dataSize, hasSubchunks);

        if (file.Position() == dataSize + sizeof(chunkType) + sizeof(chunkSizeRaw)) {
            GAMEMODELLOG_DEBUG("File '%s': Read past end of chunk data (type: 0x%X).", file.Get_File_Name().Str(), chunkType);
            return true;
        }

        // If we've read the entire chunk, exit loop (we are done with the current chunk)
        if (file.Position() == currentPosition + dataSize) break; // endPosition == dataSize
    }
    return true;
}

bool GameModelFile::Write_W3D_Chunks(FileRef &file, const ChunkInfos &parentChunks)
{
    for (const auto &chunk : parentChunks) {
        uint32_t chunkSize = (uint32_t)chunk.chunkSize;
        bool hasSubchunks = !chunk.subChunks.empty();

        if (hasSubchunks) {
            chunkSize |= 0x80000000;  // Set MSB to indicate subChunks
        }

        // Write ChunkType and ChunkSize
        if (file.Write(&chunk.chunkType, sizeof(chunk.chunkType)) != sizeof(chunk.chunkType) ||
            file.Write(&chunkSize, sizeof(chunkSize)) != sizeof(chunkSize)) {
            GAMEMODELLOG_ERROR("File '%s': Failed to write chunk header (type: 0x%X).\n", file.Get_File_Name().Str(), chunk.chunkType);
            return false;
        }

        // Write chunk data (only if there's data to write)
        if (chunk.data.size() > 0) {
            if (file.Write(chunk.data.data(), chunk.data.size()) != chunk.data.size()) {
                GAMEMODELLOG_ERROR("File '%s': Failed to write chunk data (type: 0x%X).\n", file.Get_File_Name().Str(), chunk.chunkType);
                return false;
            }
        }

        // If there are subChunks, write them recursively
        if (hasSubchunks) {
            if (!Write_W3D_Chunks(file, chunk.subChunks)) {
                return false;
            }
        }
    }

    return true;
}


void GameModelFile::Parse_Model_Data(const ChunkInfos& chunks) {
    for (const auto& chunk : chunks) {
        switch (chunk.chunkType) {
            case W3D_CHUNK_MESH:
                Parse_Mesh_Chunk(chunk);
                break;
            case W3D_CHUNK_EMITTER:
                Parse_Emitter_Chunk(chunk);
                break;
            // Add additional cases for other chunk types
            default:
                GAMEMODELLOG_DEBUG("Ignoring unknown top-level chunk type: 0x%X", chunk.chunkType);
                break;
        }
    }
}

void GameModelFile::Parse_Mesh_Chunk(const ChunkInfo& meshChunk) {
    // Example: Parse Mesh Header
    for (const auto& subChunk : meshChunk.subChunks) {
        if (subChunk.chunkType == W3D_CHUNK_MESH_HEADER3) {
            //Use the helper functions from gamemodelcommon.h
            m_meshData.header = ReadFromChunk<W3dMeshHeader3Struct>(subChunk, 0);
            break;
        }
    }
    // Parse other mesh subchunks (vertices, normals, triangles, etc.) using similar logic,
    //   referencing the struct definitions in w3d_file.h and using the helper functions from gamemodelcommon.h

    for (const auto& subChunk : meshChunk.subChunks) {
        switch (subChunk.chunkType) {
            case W3D_CHUNK_VERTICES:
                m_meshData.vertices = Thyme::ReadArrayFromChunk<W3dVectorStruct>(subChunk);
                break;
            case W3D_CHUNK_VERTEX_NORMALS:
                m_meshData.normals = Thyme::ReadArrayFromChunk<W3dVectorStruct>(subChunk);
                break;
            case W3D_CHUNK_TRIANGLES:
                m_meshData.triangles = Thyme::ReadArrayFromChunk<W3dTriStruct>(subChunk);
                break;
            case W3D_CHUNK_MESH_HEADER3:
//                m_meshData.header = Thyme::ReadFromChunk<W3dMeshHeader3>(subChunk,0);
                break;
            // ... parse other mesh subchunks ...
            default:
                break;
        }
    }
}

void GameModelFile::Parse_Emitter_Chunk(const ChunkInfo& emitterChunk) {
    //Similar logic as ParseMeshChunk, but for emitter chunks and subchunks
    for (const auto& subChunk : emitterChunk.subChunks) {
        if (subChunk.chunkType == W3D_CHUNK_EMITTER_HEADER) {
            m_emitterData.header = ReadFromChunk<W3dEmitterHeaderStruct>(subChunk, 0);
        } else if (subChunk.chunkType == W3D_CHUNK_EMITTER_INFO) {
            m_emitterData.info = ReadFromChunk<W3dEmitterInfoStruct>(subChunk, 0);
        } else if (subChunk.chunkType == W3D_CHUNK_EMITTER_INFOV2){
            m_emitterData.infoV2 = ReadFromChunk<W3dEmitterInfoStructV2>(subChunk, 0);
        }
        // ... parse other emitter subchunks ...
    }
}

const ParsedMeshData& GameModelFile::GetMeshData() const {
    return m_meshData;
}

const ParsedEmitterData& GameModelFile::GetEmitterData() const {
    return m_emitterData;
}


void GameModelFile::Log_Line(const char *prefix, const char *format, ...)
{
    FILE *file = s_logfile;

    if (file != nullptr) {
        va_list args;
        va_start(args, format);
        fputs(prefix, file);
        vfprintf(file, format, args);
        fputs("\n", file);
        va_end(args);
        fflush(file);
    }
}

} // namespace Thyme

#undef GAMEMODELLOG_TRACE
#undef GAMEMODELLOG_DEBUG
#undef GAMEMODELLOG_INFO
#undef GAMEMODELLOG_WARN
#undef GAMEMODELLOG_ERROR
#undef GAMEMODELLOG_FATAL
