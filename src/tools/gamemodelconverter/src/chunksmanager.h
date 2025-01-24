#pragma once

#include "chunkio.h"
#include "ffactory.h"
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
    ChunkList subchunks;
    ~ChunkTree() = default;
};

using ChunkTreePtr = std::unique_ptr<ChunkTree>;

struct ChunkIOFuncs
{
    const char *name;
    void (*ReadFunc)(ChunkLoadClass &chunkLoader, ChunkInfoPtr &data);
    void (*WriteFunc)(ChunkSaveClass &chunkSaver, ChunkInfoPtr &data);
};

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
    static ChunkInfoPtr createChunkInfo(
        const char *name = "Unknown",
        const StringClass &type = "Unknown",
        const StringClass &data = "Unknown",
        void *value = nullptr)
    {
        return std::make_unique<ChunkInfo>(name, type, data, value);
    }
    static void AddData(
        ChunkInfoPtr &chunk, const char *name, const StringClass &type, const StringClass &formattedValue, void *value);


    static void ParseSubchunks(ChunkLoadClass &chunkLoader, ChunkTreePtr &parentChunk);
    static void ReadChunkInfo(ChunkLoadClass &chunkLoader, ChunkPtr &chunk);
    void WriteSubchunks(ChunkSaveClass &chunkSaver, const ChunkTreePtr &parentChunk);
    void WriteChunkInfo(ChunkSaveClass &chunkSaver, const ChunkPtr &chunk);

    ChunkTreePtr & GetRootChunk() { return m_rootChunk; }

private:
    static void AddString(ChunkInfoPtr &chunk, const char *name, const StringClass &value, const StringClass &type);
    static void AddVersion(ChunkInfoPtr &chunk, uint32_t value);
    static void AddInt32(ChunkInfoPtr &chunk, const char *name, uint32_t value);
    static void AddInt16(ChunkInfoPtr &chunk, const char *name, uint16_t value);
    static void AddInt8(ChunkInfoPtr &chunk, const char *name, uint8_t value);
    static void AddInt8Array(ChunkInfoPtr &chunk, const char *name, const uint8_t *values, int count);
    static void AddFloat(ChunkInfoPtr &chunk, const char *name, float value);
    static void AddInt32Array(ChunkInfoPtr &chunk, const char *name, const uint32_t *values, int count);
    static void AddFloatArray(ChunkInfoPtr &chunk, const char *name, const float *values, int count);
    static void AddVector(ChunkInfoPtr &chunk, const char *name, const W3dVectorStruct *value);
    static void AddQuaternion(ChunkInfoPtr &chunk, const char *name, const W3dQuaternionStruct *value);
    static void AddRGB(ChunkInfoPtr &chunk, const char *name, const W3dRGBStruct *value);
    static void AddRGBArray(ChunkInfoPtr &chunk, const char *name, const W3dRGBStruct *values, int count);
    static void AddRGBA(ChunkInfoPtr &chunk, const char *name, const W3dRGBAStruct *value);
    static void AddTexCoord(ChunkInfoPtr &chunk, const char *name, const W3dTexCoordStruct *value);
    static void AddTexCoordArray(ChunkInfoPtr &chunk, const char *name, const W3dTexCoordStruct *values, int count);
    static void AddShader(ChunkInfoPtr &chunk, const char *name, const W3dShaderStruct *value);
    static void AddPS2Shader(ChunkInfoPtr &chunk, const char *name, const W3dPS2ShaderStruct *value);
    static void AddIJK(ChunkInfoPtr &chunk, const char *name, const Vector3i *value);
    static void AddIJK16(ChunkInfoPtr &chunk, const char *name, const Vector3i16 *value);

    ChunkTreePtr m_rootChunk;
    FileClass *m_file;
};
