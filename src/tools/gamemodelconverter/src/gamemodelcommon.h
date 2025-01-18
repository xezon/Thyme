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
#include "w3d_file.h"
#include <memory>
#include <string>
#include <vector>

namespace Thyme
{

struct ChunkInfo
{
    uint32_t id;
    size_t size;
    std::vector<uint8_t> data;

    //Helper function to read data from the chunk
    template<typename T>
    T* GetData() {
        if (data.size() < sizeof(T)) return nullptr; //Check for size before cast.
        return reinterpret_cast<T*>(data.data());
    }

    //Helper function to read a string from the chunk. Assumes null termination
    std::string GetString() {
        auto cstr = reinterpret_cast<const char*>(data.data());
        return std::string(cstr);
    }
};

struct ChunkInfos
{
    std::vector<ChunkInfo> chunks;

    //Helper to find a chunk by ID
    ChunkInfo* FindChunk(uint32_t id) {
        for (auto& chunk : chunks) {
            if (chunk.id == id) return &chunk;
        }
        return nullptr;
    }

    const ChunkInfo* FindChunk(uint32_t id) const {
        for (const auto& chunk : chunks) {
            if (chunk.id == id) return &chunk;
        }
        return nullptr;
    }
};

} // namespace Thyme