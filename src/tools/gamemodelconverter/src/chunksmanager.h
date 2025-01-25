#pragma once

#include "chunkio.h"
#include "ffactory.h"
#include "vector2.h"
#include "vector3.h"
#include "vector3i.h"
#include "w3d_file.h"
#include "w3d_obsolete.h"
#include <map>
#include <memory>
#include <vector>

struct ChunkInfo
{
    StringClass name; // The chunk name (e.g. "W3D_CHUNK_VERTEX_MATERIALS")
    StringClass type; // The chunk type (e.g., "W3dVectorStruct," "W3dColorStruct")
    StringClass data; // Formatted data (e.g. "Version 4.2", "X: 1, Y: 3, Z: 8"), will W3X support.
    void *value;      // The Raw type data (e.g. address of W3dVectorStruct content)

    ChunkInfo(const char *name, const StringClass &type, const StringClass &data, void *value) :
        name(name), type(type), data(data), value(value) {}
};


using ChunkInfoPtr = std::unique_ptr<ChunkInfo>;

struct Chunk
{
    uint32_t chunkType;
    uint32_t chunkSize;
    ChunkInfoPtr info;
};

using ChunkPtr = std::unique_ptr<Chunk>;
using ChunkList = std::vector<ChunkPtr>;

struct ChunkTree
{
    ChunkPtr data;
    std::vector<std::unique_ptr<ChunkTree>> subchunks;
};

using ChunkTreePtr = std::unique_ptr<ChunkTree>;

struct ChunkIOFuncs
{
    const char *name;
    void (*ReadFunc)(ChunkLoadClass &chunkLoader, ChunkTreePtr &data);
    void (*WriteFunc)(ChunkSaveClass &chunkSaver, ChunkTreePtr &data);
};

extern std::map<int, ChunkIOFuncs> chunkFuncMap;

class ChunkManager
{
public:
    // flags for the chunk manager
    enum ChunkManagerType
    {
        LOAD = 0,
        SAVE = 1,
    };

    ChunkManager(const char *filePath, int flag);
    ~ChunkManager();


    // Utility functions
    void InitiateChunkFuncMap();
    static ChunkInfoPtr createChunkInfo(
        const char *name = "Unknown",
        const StringClass &type = "Unknown",
        const StringClass &data = "Unknown",
        void *value = nullptr)
    {
        return std::make_unique<ChunkInfo>(name, type, data, value);
    }

    static void ParseSubchunks(ChunkLoadClass &chunkLoader, ChunkTreePtr &parentChunk);
    static void ReadChunkInfo(ChunkLoadClass &chunkLoader, ChunkTreePtr &chunk);
    void WriteSubchunks(ChunkSaveClass &chunkSaver, ChunkTreePtr &parentChunk);
    void WriteChunkInfo(ChunkSaveClass &chunkSaver, ChunkTreePtr &chunk);

    ChunkTreePtr & GetRootChunk() { return m_rootChunk; }
    void SetRootChunk(ChunkTreePtr root) { m_rootChunk = std::move(root); }

private:

    ChunkTreePtr m_rootChunk;
    FileClass *m_file;
};
