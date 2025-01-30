/**
 * @file  gamemodelcommon.h
 *
 * @author DevGeniusCode
 *
 * @brief Common structures for Game Model Files.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "asciistring.h"
#include "bittype.h"
#include "unicodestring.h"
#include "utility/array.h"
#include "utility/arrayview.h"
#include "w3d_file.h"
#include <memory>
#include <string>
#include <vector>

// TODO size/count of chunks
constexpr size_t g_chunkCount = 77;

namespace Thyme
{

/*
// Type aliases to avoid duplication
using W3dVector = W3dVectorStruct;
using W3dQuaternion = W3dQuaternionStruct;
using W3dRGB = W3dRGBStruct;
using W3dRGBA = W3dRGBAStruct;
using W3dMaterial3 = W3dMaterial3Struct;
using W3dVertexMaterial = W3dVertexMaterialStruct;
using W3dTri = W3dTriStruct;
using W3dVertInf = W3dVertInfStruct;
using W3dTexCoord = W3dTexCoordStruct;
using W3dMaterialInfo = W3dMaterialInfoStruct;
using W3dShader = W3dShaderStruct;
using W3dTextureInfo = W3dTextureInfoStruct;
using W3dMeshHeader3 = W3dMeshHeader3Struct;
using W3dMeshAABTreeHeader = W3dMeshAABTreeHeader;
using W3dMeshAABTreeNode = W3dMeshAABTreeNode;
using W3dEmitterHeader = W3dEmitterHeaderStruct;
using W3dEmitterInfo = W3dEmitterInfoStruct;
using W3dEmitterInfoV2 = W3dEmitterInfoStructV2;
using W3dEmitterProperty = W3dEmitterPropertyStruct;
// ...add other type aliases as needed...

 */

struct ChunkInfo {
    uint32_t chunkType;           // Chunk ID (from w3d_file.h)
    uint32_t chunkSize;           // Size of chunk data (including subchunk flag) Note: MSB indicates presence of subchunks.
    std::vector<uint8_t> data;     // Raw chunk data
    std::vector<ChunkInfo> subChunks;         // Vector to store sub-chunks
};


using ChunkInfos = std::vector<ChunkInfo>;
using ChunkInfosArray = rts::array<ChunkInfos, g_chunkCount>;
using ConstChunkInfosPtrArray = rts::array<const ChunkInfos *, g_chunkCount>;
using ChunkInfosPtrArray = rts::array<ChunkInfos *, g_chunkCount>;
using Utf8Array = std::vector<uint8_t>;
using utf8view = rts::array_view<uint8_t>;

struct ParsedMeshData {
    std::vector<W3dVectorStruct> vertices;
    std::vector<W3dVectorStruct> normals;
    std::vector<W3dTriStruct> triangles;
    // ... other mesh data ...
    W3dMeshHeader3Struct header;
};

struct ParsedEmitterData {
    W3dEmitterHeaderStruct header;
    W3dEmitterInfoStruct info;
    W3dEmitterInfoStructV2 infoV2;
    std::vector<W3dEmitterPropertyStruct> properties;
};

//Helper functions for reading data from chunk
template <typename T>
T ReadFromChunk(const Thyme::ChunkInfo& chunk, size_t offset) {
    if (offset + sizeof(T) > chunk.data.size()) {
//        std::cerr << "Error: Trying to read past the end of the chunk data!" << std::endl;
        T temp{};
        return temp;
    }
    return *reinterpret_cast<const T*>(&chunk.data[offset]);
}

template <typename T>
std::vector<T> ReadArrayFromChunk(const Thyme::ChunkInfo& chunk) {
    size_t numElements = chunk.data.size() / sizeof(T);
    std::vector<T> array(numElements);
    if (chunk.data.size() % sizeof(T) != 0) {
//        std::cerr << "Error: Chunk data size is not a multiple of element size!" << std::endl;
        return {}; //Return empty vector in case of error
    }
    memcpy(array.data(), chunk.data.data(), chunk.data.size());
    return array;
}

} // namespace Thyme