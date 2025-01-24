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
#include "gamemodelfile.h"


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
// TODO delete this function
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
    m_options(GameModelOption::NONE), m_rootChunk(nullptr), m_chunkLoader(nullptr) {};

bool GameModelFile::Is_Loaded() const
{
    return false;
}

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

    bool success = false;

    switch (filetype) {

        case FileType::W3D: {
            success = Read_W3D_File(filename, m_options);
            break;
        }
        case FileType::W3X:
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


    bool success = false;

    switch (filetype) {

        case FileType::W3D: {
            success = Write_W3D_File(filename, m_options);
            break;
        }
        case FileType::W3X:
        case FileType::BLEND:
        case FileType::MAX: {
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
    m_options = Options::Value::NONE;
    // TODO good free memory?
//    if (m_chunkManager) {
//        delete m_chunkManager;
//        m_chunkManager = nullptr;
//    }
}

bool GameModelFile::Read_W3D_File(const char *filename, const Options& options)
{
    GAMEMODELLOG_INFO("Reading model file '%s' in W3D format", filename);



    auto_file_ptr file(g_theFileFactory, filename);
    if (!file->Open(FileOpenType::FM_READ)) {
        GAMEMODELLOG_ERROR("Could not open file: %s", filename);
        return false;
    }
    m_chunkManager = new ChunkManager(filename, ChunkManager::ChunkManagerType::LOAD);
    m_chunkLoader = ChunkLoadClass(file);
    m_chunkManager->ParseSubchunks(m_chunkLoader, m_chunkManager->GetRootChunk());
//    if (!Log_Unknown_Chunks(m_rootChunk)) {
//        GAMEMODELLOG_ERROR("Unknown chunks found in file: %s", filename);
//        GAMEMODELLOG_ERROR("Failed to load chunks correctly");
//        return false;
//    }
//    else {
    GAMEMODELLOG_INFO("Successfully parsed W3D file: %s", filename);

    return true;
}

bool GameModelFile::Write_W3D_File(const char *filename, const Options& options)
{
    GAMEMODELLOG_INFO("Writing model file '%s' in W3D format", filename);

    auto_file_ptr file(g_theFileFactory, filename);
    if (!file->Open(FileOpenType::FM_WRITE)) {
        GAMEMODELLOG_ERROR("Could not open file: %s", filename);
        return false;
    }
    ChunkSaveClass chunkSaver(file);
    m_chunkManager->WriteSubchunks(chunkSaver, m_chunkManager->GetRootChunk());
    return true;
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
