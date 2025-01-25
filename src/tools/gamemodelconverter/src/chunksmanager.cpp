#include "chunksmanager.h"

ChunkManager::ChunkManager(const char *filePath, int flag)
{
    m_rootChunk = std::make_unique<ChunkTree>();
    InitiateChunkFuncMap();
}

ChunkManager::~ChunkManager()
{
    if (m_file) {
        m_file->Close();
    }
}

void ChunkManager::ParseSubchunks(ChunkLoadClass &chunkLoader, ChunkTreePtr &parentChunk)
{
    if (parentChunk == nullptr) {
        parentChunk = std::make_unique<ChunkTree>();
    }
    while (chunkLoader.Open_Chunk()) {
        if (chunkLoader.Contains_Chunks()) {
            while (chunkLoader.Open_Chunk()) {
                ChunkTreePtr subchunk = std::make_unique<ChunkTree>();
                ReadChunkInfo(chunkLoader, subchunk);
                parentChunk->subchunks.push_back(std::move(subchunk));
                chunkLoader.Close_Chunk();
            }
        }
        chunkLoader.Close_Chunk();
    }
}

void ChunkManager::ReadChunkInfo(ChunkLoadClass &chunkLoader, ChunkTreePtr &data)
{
    if (data == nullptr) {
        captainslog_error("Chunk is null");
        return;
    }
    ChunkPtr chunk = std::make_unique<Chunk>();
    chunk->chunkType = chunkLoader.Cur_Chunk_ID();
    chunk->chunkSize = chunkLoader.Cur_Chunk_Length();
    data->data = std::move(chunk);
    data->subchunks = std::vector<ChunkTreePtr>(1);
    auto it = chunkFuncMap.find(data->data->chunkType);
    if (it != chunkFuncMap.end()) {
        it->second.ReadFunc(chunkLoader, data);
    } else {
        StringClass str;
        str.Format("Unknown Chunk 0x%X", data->data->chunkType);
        captainslog_warn((const char *)str);
        chunk->info = nullptr;
    }
}

void ChunkManager::WriteSubchunks(ChunkSaveClass &chunkSaver, ChunkTreePtr &parentChunk)
{
    if (parentChunk->data) {
        if (!chunkSaver.Begin_Chunk(parentChunk->data->chunkType)) {
            captainslog_error("Failed to begin chunk");
            return;
        }
        WriteChunkInfo(chunkSaver, parentChunk);
        if (!chunkSaver.End_Chunk()) {
            captainslog_error("Failed to end chunk");
            return;
        }

    }

    for (auto& subchunk : parentChunk->subchunks) {
        WriteChunkInfo(chunkSaver, subchunk);
    }
}

void ChunkManager::WriteChunkInfo(ChunkSaveClass &chunkSaver, ChunkTreePtr &data)
{
    auto it = chunkFuncMap.find(data->data->chunkType);
    if (it != chunkFuncMap.end()) {
        it->second.WriteFunc(chunkSaver, data);
    } else {
        StringClass str;
        str.Format("Unknown Chunk 0x%X", data->data->chunkType);
        captainslog_warn((const char*)str);
    }
}
