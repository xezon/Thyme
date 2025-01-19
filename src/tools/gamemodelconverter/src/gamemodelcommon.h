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

struct ChunkInfo
{
    uint32_t chunkType;
    uint32_t chunkSize; // Note: MSB indicates presence of subchunks.
    std::vector<uint8_t> data;
    std::vector<ChunkInfo> subChunks; // To handle nested chunks
};

using ChunkInfos = std::vector<ChunkInfo>;
using ChunkInfosArray = rts::array<ChunkInfos, g_chunkCount>;
using ConstChunkInfosPtrArray = rts::array<const ChunkInfos *, g_chunkCount>;
using ChunkInfosPtrArray = rts::array<ChunkInfos *, g_chunkCount>;
using Utf8Array = std::vector<uint8_t>;
using utf8view = rts::array_view<uint8_t>;

// Helper function to read a string from a ChunkInfo's data
inline std::string readStringFromChunk(const ChunkInfo& chunk) {
    if (chunk.data.empty()) return "";
    return std::string((char*)chunk.data.data(), chunk.data.size());
}

} // namespace Thyme