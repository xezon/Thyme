#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#include "chunksmanager.h"
#include <string>
#include <typeinfo>

// Helper Functions

char *ReadChunkRaw(ChunkLoadClass &cload)
{
    if (!cload.Cur_Chunk_Length()) {
        char *c = new char[1];
        c[0] = 0;
        return c;
    }
    char *c = new char[cload.Cur_Chunk_Length()];
    size_t bytes = cload.Read(c, cload.Cur_Chunk_Length());
    captainslog_dbgassert(cload.Cur_Chunk_Length() == bytes, "Read chunk size does not match data size");
    return c;
}

void WriteChunkRaw(ChunkSaveClass &csave, unsigned id, const void *data, unsigned dataSize)
{
    csave.Begin_Chunk(id);
    size_t bytes = csave.Write(data, dataSize);
    captainslog_dbgassert(dataSize == bytes, "Write chunk size does not match data size");
    csave.End_Chunk();
}

template<typename T> void WriteArray(ChunkSaveClass &csave, unsigned id, unsigned dataSize, unsigned numElements)
{
    csave.Begin_Chunk(id);

    for (unsigned i = 0; i < numElements; i++) {
        csave.Write(&arrayData[i], dataSize);
    }

    csave.End_Chunk();
}

void AddData(ChunkTreePtr &data, const char *name, const StringClass &type, const StringClass &formattedValue, void *value)
{
    if (data->data->info == nullptr) {
        data->data->info = std::move(ChunkManager::createChunkInfo(name, type, formattedValue, value));
    } else {
        captainslog_error("ChunkInfo %s already exists, cannot set %s", (const char *)data->data->info->name, name);
    }
}

void AddString(ChunkTreePtr &data, const char *name, const StringClass &string, const StringClass &type)
{
    AddData(data, name, type, string, (void *)&string);
}

void AddInt32(ChunkTreePtr &data, const char *name, uint32_t value)
{
    char c[256];
    sprintf(c, "%u", value);
    AddData(data, name, "uint32_t", c, (void *)&value);
}

void AddInt8(ChunkTreePtr &data, const char *name, uint8_t value)
{
    char c[256];
    sprintf(c, "%hhu", value);
    AddData(data, name, "uint8_t", c, (void *)&value);
}

void AddFloat(ChunkTreePtr &data, const char *name, float value)
{
    char c[256];
    sprintf(c, "%f", value);
    AddData(data, name, "float", c, (void *)&value);
}

void AddVector(ChunkTreePtr &data, const char *name, const W3dVectorStruct *value)
{
    char c[256];
    sprintf(c, "%f %f %f", value->x, value->y, value->z);
    AddData(data, name, "W3dVectorStruct", c, (void *)value);
}

// Macro for Defining Functions

// Macro for Defining Functions
// READ_WRITE_CHUNK_STRING
template<typename> void Read_Chunk_String(ChunkLoadClass &cload, ChunkTreePtr &data, const char *id)
{
    char* rawData = ReadChunkRaw(cload);
    if (strlen(rawData) == 0) { // case of null terminator
        for (unsigned i = 0; i < data->data->chunkSize; i++) {
            captainslog_dbgassert(rawData[i] == '\0', "Chunk %s size does not match data size", id);
        }
    } else {
        captainslog_dbgassert(data->data->chunkSize == strlen(rawData) + 1, "Chunk %s size does not match data size", id);
    }
    AddData(data, id, "string", rawData, (void *)rawData);
}

template<typename> void Write_Chunk_String(ChunkSaveClass &csave, ChunkTreePtr &data, unsigned id)
{
    char* rawData = (char*)data->data->info->value;
    if (strlen(rawData) == 0) { // case of null terminator
        for (unsigned i = 0; i < data->data->chunkSize; i++) {
            captainslog_dbgassert(rawData[i] == '\0', "Chunk %s size does not match data size", id);
        }
    } else {
        captainslog_dbgassert(data->data->chunkSize == strlen(rawData) + 1, "Chunk %s size does not match data size", id);
    }
    WriteChunkRaw(csave, id, rawData, data->data->chunkSize);
}
#define READ_WRITE_CHUNK_STRING(id) \
    void Read_##id(ChunkLoadClass &cload, ChunkTreePtr &data) { Read_Chunk_String<StringClass>(cload, data, #id); } \
    void Write_##id(ChunkSaveClass &csave, ChunkTreePtr &data) { Write_Chunk_String<StringClass>(csave, data, id); } \

// READ_WRITE_CHUNK
template<typename T> void Read_Chunk_Struct(ChunkLoadClass &cload, ChunkTreePtr &data, const char *id)
{
    char* rawData = ReadChunkRaw(cload);
    captainslog_dbgassert(data->data->chunkSize == sizeof(T), "Chunk %s size does not match data size", id);
    AddData(data, id, typeid(T).name(), "", (void *)rawData);
}

template<typename T> void Write_Chunk_Struct(ChunkSaveClass &csave, ChunkTreePtr &data, unsigned id)
{
    char *rawData = (char *)data->data->info->value;
    captainslog_dbgassert(data->data->chunkSize == sizeof(T), "Chunk %X size does not match data size", id);
    WriteChunkRaw(csave, id, rawData, sizeof(T));
}
#define READ_WRITE_CHUNK(id, type) \
    void Read_##id(ChunkLoadClass &cload, ChunkTreePtr &data) { Read_Chunk_Struct<type>(cload, data, #id); } \
    void Write_##id(ChunkSaveClass &csave, ChunkTreePtr &data) { Write_Chunk_Struct<type>(csave, data, id); } \

// READ_WRITE_CHUNK_ARRAY
template<typename T> void Read_Chunk_Array(ChunkLoadClass &cload, ChunkTreePtr &data, const char *id)
{
    char *rawData = ReadChunkRaw(cload);
    captainslog_dbgassert(data->data->chunkSize % sizeof(T) == 0, "Chunk %s size does not match data size", id);
    std::vector<T> *arrayData = new std::vector<T>();
    unsigned numElements = data->data->chunkSize / sizeof(T);

    for (unsigned i = 0; i < numElements; i++) {
        arrayData->push_back(*reinterpret_cast<T*>(rawData + i * sizeof(T)));
    }

    AddData(data, id, (std::string(typeid(T).name()) + "[]").c_str(), "", (void *)arrayData);
    delete [] rawData;
}


template<typename T> void Write_Chunk_Array(ChunkSaveClass &csave, ChunkTreePtr &data, unsigned id)
{
    std::vector<T>* arrayData = reinterpret_cast<std::vector<T>*>(data->data->info->value);
    captainslog_dbgassert(data->data->chunkSize % sizeof(T) == 0, "Chunk %X size does not match data size", id);
    unsigned numElements = data->data->chunkSize / sizeof(T);
    char *chunkData = new char[numElements * sizeof(T)];
    for (unsigned i = 0; i < numElements; i++) {
        memcpy(chunkData + i * sizeof(T), &(*arrayData)[i], sizeof(T));
    }
    WriteChunkRaw(csave, id, chunkData, numElements * sizeof(T));

    delete[] chunkData;
}
#define READ_WRITE_CHUNK_ARRAY(id, type) \
    void Read_##id(ChunkLoadClass &cload, ChunkTreePtr &data) { Read_Chunk_Array<type>(cload, data, #id); } \
    void Write_##id(ChunkSaveClass &csave, ChunkTreePtr &data) { Write_Chunk_Array<type>(csave, data, id); } \

// READ_WRITE_SUBCHUNKS
template<typename T> void Read_Sub_Chunks(ChunkLoadClass &cload, ChunkTreePtr &data, const char *id)
{
    AddData(data, id, "StringClass", id, nullptr);
    ChunkManager::ReadSubChunks(cload, data);
}

template<typename T> void Write_Sub_Chunks(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    ChunkManager::WriteSubChunks(csave, data);
}
#define READ_WRITE_SUBCHUNKS(id) \
    void Read_##id(ChunkLoadClass &cload, ChunkTreePtr &data) { Read_Sub_Chunks<StringClass>(cload, data, #id); } \
    void Write_##id(ChunkSaveClass &csave, ChunkTreePtr &data) { Write_Sub_Chunks<StringClass>(csave, data); }    \


// Read and Write Functions
READ_WRITE_CHUNK_ARRAY(O_W3D_CHUNK_MATERIALS, W3dMaterialStruct)

READ_WRITE_CHUNK_ARRAY(O_W3D_CHUNK_MATERIALS2, W3dMaterial2Struct)

void Read_O_W3D_CHUNK_POV_QUADRANGLES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_POV_QUADRANGLES is unsupported");
    char *rawData = ReadChunkRaw(cload);
    AddData(data, "O_W3D_CHUNK_POV_QUADRANGLES", "char", "string", (void *)rawData);
}

void Write_O_W3D_CHUNK_POV_QUADRANGLES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_POV_QUADRANGLES is unsupported");
    char *rawData = (char *)data->data->info->value;
    WriteChunkRaw(csave, O_W3D_CHUNK_POV_QUADRANGLES, rawData, data->data->chunkSize);
}

void Read_O_W3D_CHUNK_POV_TRIANGLES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_POV_TRIANGLES is unsupported");
    char *rawData = ReadChunkRaw(cload);
    AddData(data, "O_W3D_CHUNK_POV_TRIANGLES", "char", "string", (void *)rawData);
}

void Write_O_W3D_CHUNK_POV_TRIANGLES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_POV_TRIANGLES is unsupported");
    char *rawData = (char *)data->data->info->value;
    WriteChunkRaw(csave, O_W3D_CHUNK_POV_TRIANGLES, rawData, data->data->chunkSize);
}

void Read_O_W3D_CHUNK_QUADRANGLES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_QUADRANGLES is outdated");
    char *rawData = ReadChunkRaw(cload);
    AddData(data, "O_W3D_CHUNK_QUADRANGLES", "char", "string", (void *)rawData);
}

void Write_O_W3D_CHUNK_QUADRANGLES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_QUADRANGLES is outdated");
    char *rawData = (char *)data->data->info->value;
    WriteChunkRaw(csave, O_W3D_CHUNK_QUADRANGLES, rawData, data->data->chunkSize);
}

READ_WRITE_CHUNK_ARRAY(O_W3D_CHUNK_SURRENDER_TRIANGLES, W3dSurrenderTriangleStruct)

void Read_O_W3D_CHUNK_TRIANGLES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_TRIANGLES is obsoleted");
    char *rawData = ReadChunkRaw(cload);
    AddData(data, "O_W3D_CHUNK_TRIANGLES", "char", "string", (void *)rawData);
}

void Write_O_W3D_CHUNK_TRIANGLES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_TRIANGLES is obsoleted");
    char *rawData = (char *)data->data->info->value;
    WriteChunkRaw(csave, O_W3D_CHUNK_TRIANGLES, rawData, data->data->chunkSize);
}

READ_WRITE_CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_COLOR_KEYFRAME, W3dEmitterColorKeyframeStruct)

READ_WRITE_CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME, W3dEmitterOpacityKeyframeStruct)

READ_WRITE_CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_SIZE_KEYFRAME, W3dEmitterSizeKeyframeStruct)

READ_WRITE_CHUNK(OBSOLETE_W3D_CHUNK_SHADOW_NODE, W3dHModelNodeStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_AABTREE)

READ_WRITE_CHUNK(W3D_CHUNK_AABTREE_HEADER, W3dMeshAABTreeHeader)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_AABTREE_NODES, W3dMeshAABTreeNode)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_AABTREE_POLYINDICES, uint32_t)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_AGGREGATE)

READ_WRITE_CHUNK(W3D_CHUNK_AGGREGATE_CLASS_INFO, W3dAggregateMiscInfo)

READ_WRITE_CHUNK(W3D_CHUNK_AGGREGATE_HEADER, W3dAggregateHeaderStruct)

void Read_W3D_CHUNK_AGGREGATE_INFO(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    W3dAggregateInfoStruct *info = (W3dAggregateInfoStruct *)rawData;
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dAggregateInfoStruct) + info->SubobjectCount * sizeof(W3dAggregateSubobjectStruct),
        "Chunk W3D_CHUNK_AGGREGATE_INFO size does not match expected size for AggregateInfo structure");
    AddData(data, "W3D_CHUNK_AGGREGATE_INFO", "W3dAggregateInfoStruct[][]", "Obsolete", (void *)rawData);
}

void Write_W3D_CHUNK_AGGREGATE_INFO(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    char *rawData = (char *)data->data->info->value;
    WriteChunkRaw(csave, W3D_CHUNK_AGGREGATE_INFO, rawData, data->data->chunkSize);
}

READ_WRITE_SUBCHUNKS(W3D_CHUNK_ANIMATION)

void Read_W3D_CHUNK_ANIMATION_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    W3dAnimChannelStruct *channel = (W3dAnimChannelStruct *)rawData;

    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dAnimChannelStruct) +
                (channel->LastFrame - channel->FirstFrame) * channel->VectorLen * sizeof(float),
        "Chunk W3D_CHUNK_ANIMATION_CHANNEL size does not match data size");

    if (channel->Flags > ANIM_CHANNEL_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    AddData(data, "W3D_CHUNK_ANIMATION_CHANNEL", "W3dAnimChannelStruct", "", (void *)rawData);
}

void Write_W3D_CHUNK_ANIMATION_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dAnimChannelStruct *channel = (W3dAnimChannelStruct *)data->data->info->value;
    if (channel->Flags > ANIM_CHANNEL_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    WriteChunkRaw(csave, W3D_CHUNK_ANIMATION_CHANNEL, channel, sizeof(W3dAnimChannelStruct));
}

READ_WRITE_CHUNK(W3D_CHUNK_ANIMATION_HEADER, W3dAnimHeaderStruct)

void Read_W3D_CHUNK_BIT_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    W3dBitChannelStruct *channel = (W3dBitChannelStruct *)rawData;
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dBitChannelStruct)
                + (channel->LastFrame - channel->FirstFrame) * sizeof(bool),
        "Chunk W3D_CHUNK_BIT_CHANNEL size does not match data size");
    if (channel->Flags > BIT_CHANNEL_TIMECODED_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_BIT_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    AddData(data, "W3D_CHUNK_BIT_CHANNEL", "W3dBitChannelStruct[]", "", (void *)channel);
}
void Write_W3D_CHUNK_BIT_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dBitChannelStruct *channel = (W3dBitChannelStruct *)data->data->info->value;
    if (channel->Flags > BIT_CHANNEL_TIMECODED_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_BIT_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    WriteChunkRaw(csave, W3D_CHUNK_BIT_CHANNEL, channel, data->data->chunkSize);
}

void Read_W3D_CHUNK_BOX(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dBoxStruct), "Chunk W3D_CHUNK_BOX size does not match data size");
    W3dBoxStruct *box = (W3dBoxStruct *)rawData;

    if (box->Attributes & 4) {
        StringClass str;
        str.Format("W3D_CHUNK_BOX Unknown Attribute 0x00000004");
        captainslog_warn((const char *)str);
    }

    if (box->Attributes & 8) {
        StringClass str;
        str.Format("W3D_CHUNK_BOX Unknown Attribute 0x00000008");
        captainslog_warn((const char *)str);
    }

    if (box->Attributes & 0xFFFFFE00) {
        StringClass str;
        str.Format("W3D_CHUNK_BOX Unknown Attributes %x", box->Attributes & 0xFFFFFE00);
        captainslog_warn((const char *)str);
    }

    AddData(data, "W3D_CHUNK_BOX", "W3dBoxStruct", "", (void *)rawData);
}

void Write_W3D_CHUNK_BOX(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dBoxStruct *box = (W3dBoxStruct *)data->data->info->value;
    if (box->Attributes & 4) {
        StringClass str;
        str.Format("W3D_CHUNK_BOX Unknown Attribute 0x00000004");
        captainslog_warn((const char *)str);
    }

    if (box->Attributes & 8) {
        StringClass str;
        str.Format("W3D_CHUNK_BOX Unknown Attribute 0x00000008");
        captainslog_warn((const char *)str);
    }

    if (box->Attributes & 0xFFFFFE00) {
        StringClass str;
        str.Format("W3D_CHUNK_BOX Unknown Attributes %x", box->Attributes & 0xFFFFFE00);
        captainslog_warn((const char *)str);
    }

    WriteChunkRaw(csave, W3D_CHUNK_BOX, box, sizeof(W3dBoxStruct));
}

READ_WRITE_SUBCHUNKS(W3D_CHUNK_COLLECTION)

READ_WRITE_CHUNK(W3D_CHUNK_COLLECTION_HEADER, W3dCollectionHeaderStruct)

READ_WRITE_CHUNK_STRING(W3D_CHUNK_COLLECTION_OBJ_NAME)

READ_WRITE_CHUNK(W3D_CHUNK_COLLISION_NODE, W3dHModelNodeStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_DAMAGE)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_DAMAGE_COLORS, W3dDamageColorStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_DAMAGE_HEADER, W3dDamageStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_DAMAGE_VERTICES, W3dDamageVertexStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_DAZZLE)

READ_WRITE_CHUNK_STRING(W3D_CHUNK_DAZZLE_NAME)

READ_WRITE_CHUNK_STRING(W3D_CHUNK_DAZZLE_TYPENAME)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_DCG, W3dRGBAStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_DIG, W3dRGBStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_EMITTER)

void Read_W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    W3dEmitterBlurTimeHeaderStruct *header = (W3dEmitterBlurTimeHeaderStruct *)rawData;
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dEmitterBlurTimeHeaderStruct)
                + (header->KeyframeCount + 1) * sizeof(W3dEmitterBlurTimeKeyframeStruct),
        "Chunk W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES size does not match data size");

    AddData(data, "W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES", "W3dEmitterBlurTimeHeaderStruct[][]", "", (void *)rawData);
}

void Write_W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    char *rawData = (char *)data->data->info->value;
    WriteChunkRaw(csave, W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES, rawData, data->data->chunkSize);
}

void Read_W3D_CHUNK_EMITTER_FRAME_KEYFRAMES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    W3dEmitterFrameHeaderStruct *header = (W3dEmitterFrameHeaderStruct *)rawData;
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dEmitterFrameHeaderStruct)
                + (header->KeyframeCount + 1) * sizeof(W3dEmitterFrameKeyframeStruct),
        "Chunk W3D_CHUNK_EMITTER_FRAME_KEYFRAMES size does not match data size");

    AddData(data, "W3D_CHUNK_EMITTER_FRAME_KEYFRAMES", "W3dEmitterFrameHeaderStruct[][]", "", (void *)rawData);
}

void Write_W3D_CHUNK_EMITTER_FRAME_KEYFRAMES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    char *rawData = (char *)data->data->info->value;
    WriteChunkRaw(csave, W3D_CHUNK_EMITTER_FRAME_KEYFRAMES, rawData, data->data->chunkSize);
}

READ_WRITE_CHUNK(W3D_CHUNK_EMITTER_HEADER, W3dEmitterHeaderStruct)

READ_WRITE_CHUNK(W3D_CHUNK_EMITTER_INFO, W3dEmitterInfoStruct)

READ_WRITE_CHUNK(W3D_CHUNK_EMITTER_INFOV2, W3dEmitterInfoStructV2)

void Read_W3D_CHUNK_EMITTER_PROPS(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    W3dEmitterPropertyStruct *props = (W3dEmitterPropertyStruct *)rawData;

    W3dEmitterColorKeyframeStruct *color = (W3dEmitterColorKeyframeStruct *)(rawData + sizeof(W3dEmitterPropertyStruct));

    W3dEmitterOpacityKeyframeStruct *opacity = (W3dEmitterOpacityKeyframeStruct *)(rawData
        + sizeof(W3dEmitterPropertyStruct) + (props->ColorKeyframes * sizeof(W3dEmitterColorKeyframeStruct)));

    W3dEmitterSizeKeyframeStruct *size = (W3dEmitterSizeKeyframeStruct *)(rawData + sizeof(W3dEmitterPropertyStruct)
        + (props->ColorKeyframes * sizeof(W3dEmitterColorKeyframeStruct))
        + (props->OpacityKeyframes * sizeof(W3dEmitterOpacityKeyframeStruct)));
    captainslog_dbgassert(data->data->chunkSize ==
            sizeof(W3dEmitterPropertyStruct)
                + (props->ColorKeyframes * sizeof(W3dEmitterColorKeyframeStruct))
                + (props->OpacityKeyframes * sizeof(W3dEmitterOpacityKeyframeStruct))
                + (props->SizeKeyframes * sizeof(W3dEmitterSizeKeyframeStruct)),
        "Chunk W3D_CHUNK_EMITTER_PROPS size does not match data size");


    AddData(data, "W3D_CHUNK_EMITTER_PROPS", "W3dEmitterPropertyStruct[][][][]", "", (void *)rawData);
}

void Write_W3D_CHUNK_EMITTER_PROPS(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    char *rawData = (char *)data->data->info->value;
    WriteChunkRaw(csave, W3D_CHUNK_EMITTER_PROPS, rawData, data->data->chunkSize);
}

void Read_W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    W3dEmitterRotationHeaderStruct *header = (W3dEmitterRotationHeaderStruct *)rawData;
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dEmitterRotationHeaderStruct)
                + (header->KeyframeCount + 1) * sizeof(W3dEmitterRotationKeyframeStruct),
        "Chunk W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES size does not match data size");

    AddData(data, "W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES", "W3dEmitterRotationHeaderStruct[][]", "", (void *)rawData);
}

void Write_W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    char *rawData = (char *)data->data->info->value;
    WriteChunkRaw(csave, W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES, rawData, data->data->chunkSize);
}

READ_WRITE_CHUNK_STRING(W3D_CHUNK_EMITTER_USER_DATA)

READ_WRITE_CHUNK(W3D_CHUNK_FAR_ATTENUATION, W3dLightAttenuationStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_HIERARCHY)

READ_WRITE_CHUNK(W3D_CHUNK_HIERARCHY_HEADER, W3dHierarchyStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_HLOD)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_HLOD_AGGREGATE_ARRAY)

READ_WRITE_CHUNK(W3D_CHUNK_HLOD_HEADER, W3dHLodHeaderStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_HLOD_LOD_ARRAY)

READ_WRITE_CHUNK(W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER, W3dHLodArrayHeaderStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_HLOD_PROXY_ARRAY)

READ_WRITE_CHUNK(W3D_CHUNK_HLOD_SUB_OBJECT, W3dHLodSubObjectStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_HMODEL)

READ_WRITE_CHUNK(OBSOLETE_W3D_CHUNK_HMODEL_AUX_DATA, W3dHModelAuxDataStruct)

READ_WRITE_CHUNK(W3D_CHUNK_HMODEL_HEADER, W3dHModelHeaderStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_LIGHT)

void Read_W3D_CHUNK_LIGHT_INFO(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dLightStruct), "Chunk W3D_CHUNK_LIGHT_INFO size does not match data size");
    W3dLightStruct *light = (W3dLightStruct *)rawData;

    int type = light->Attributes & W3D_LIGHT_ATTRIBUTE_TYPE_MASK;
    if (type != W3D_LIGHT_ATTRIBUTE_POINT && type != W3D_LIGHT_ATTRIBUTE_SPOT && type != W3D_LIGHT_ATTRIBUTE_DIRECTIONAL) {
        StringClass str;
        str.Format("W3D_CHUNK_LIGHT_INFO Unknown Light Type %x", type);
        captainslog_warn((const char *)str);
    }

    if (light->Attributes & 0xFFFFFE00) {
        StringClass str;
        str.Format("W3D_CHUNK_LIGHT_INFO Unknown Light Flags %x", light->Attributes & 0xFFFFFE00);
        captainslog_warn((const char *)str);
    }
    AddData(data, "W3D_CHUNK_LIGHT_INFO", "W3dLightStruct", "", (void *)rawData);
}

void Write_W3D_CHUNK_LIGHT_INFO(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dLightStruct *light = (W3dLightStruct *)data->data->info->value;
    int type = light->Attributes & W3D_LIGHT_ATTRIBUTE_TYPE_MASK;
    if (type != W3D_LIGHT_ATTRIBUTE_POINT && type != W3D_LIGHT_ATTRIBUTE_SPOT && type != W3D_LIGHT_ATTRIBUTE_DIRECTIONAL) {
        StringClass str;
        str.Format("W3D_CHUNK_LIGHT_INFO Unknown Light Type %x", type);
        captainslog_warn((const char *)str);
    }

    if (light->Attributes & 0xFFFFFE00) {
        StringClass str;
        str.Format("W3D_CHUNK_LIGHT_INFO Unknown Light Flags %x", light->Attributes & 0xFFFFFE00);
        captainslog_warn((const char *)str);
    }

    WriteChunkRaw(csave, W3D_CHUNK_LIGHT_INFO, light, sizeof(W3dLightStruct));
}

READ_WRITE_CHUNK(W3D_CHUNK_LIGHT_TRANSFORM, W3dLightTransformStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_LIGHTSCAPE)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_LIGHTSCAPE_LIGHT)

READ_WRITE_CHUNK(W3D_CHUNK_LOD, W3dLODStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_LODMODEL)

READ_WRITE_CHUNK(W3D_CHUNK_LODMODEL_HEADER, W3dLODModelHeaderStruct)

READ_WRITE_CHUNK_STRING(W3D_CHUNK_MAP3_FILENAME)

READ_WRITE_CHUNK(W3D_CHUNK_MAP3_INFO, W3dMap3Struct)

READ_WRITE_CHUNK(W3D_CHUNK_MATERIAL_INFO, W3dMaterialInfoStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_MATERIAL_PASS)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_MATERIAL3)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_MATERIAL3_DC_MAP)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_MATERIAL3_DI_MAP)

READ_WRITE_CHUNK(W3D_CHUNK_MATERIAL3_INFO, W3dMaterial3Struct)

READ_WRITE_CHUNK_STRING(W3D_CHUNK_MATERIAL3_NAME)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_MATERIAL3_SC_MAP)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_MATERIAL3_SI_MAP)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_MATERIALS3)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_MESH)

READ_WRITE_CHUNK(W3D_CHUNK_MESH_HEADER, W3dMeshHeaderStruct)

void Read_W3D_CHUNK_MESH_HEADER3(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dMeshHeader3Struct), "Chunk W3D_CHUNK_MESH_HEADER3 size does not match data size");
    W3dMeshHeader3Struct *header = (W3dMeshHeader3Struct *)rawData;
    int type = header->Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK;
    if (type != W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL && type != W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED
        && type != W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN && type != OBSOLETE_W3D_MESH_FLAG_GEOMETRY_TYPE_SHADOW
        && type != W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX && type != W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX
        && type != W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED && type != W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_Z_ORIENTED) {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Mesh Type %x", type);
        captainslog_warn((const char *)str);
    }

    if (header->Attributes & 0x00000800) {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Attribute 0x00000800");
        captainslog_warn((const char *)str);
    }

    if (header->Attributes & W3D_MESH_FLAG_PRELIT_MASK) {
        if (!header->PrelitVersion) {
            StringClass str;
            str.Format("W3D_CHUNK_MESH_HEADER3 Unknown value of Attribute PrelitVersion: %x", header->PrelitVersion);
            captainslog_warn((const char *)str);
        }
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 N/A PrelitVersion");
        captainslog_warn((const char *)str);
    }

    if (header->VertexChannels & 0xFFFFFF00) {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Vertex Channels %x", header->VertexChannels);
        captainslog_warn((const char *)str);
    }

    if (header->FaceChannels & 0xFFFFFFFE) {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Face Channels %x", header->FaceChannels);
        captainslog_warn((const char *)str);
    }
    AddData(data, "W3D_CHUNK_MESH_HEADER3", "W3dMeshHeader3Struct", "", (void *)header);
}

void Write_W3D_CHUNK_MESH_HEADER3(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dMeshHeader3Struct *header = (W3dMeshHeader3Struct *)data->data->info->value;
    int type = header->Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK;

    if (type != W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL && type != W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED
        && type != W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN && type != OBSOLETE_W3D_MESH_FLAG_GEOMETRY_TYPE_SHADOW
        && type != W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX && type != W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX
        && type != W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED && type != W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_Z_ORIENTED) {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Mesh Type %x", type);
        captainslog_warn((const char *)str);
    }

    if (header->Attributes & 0x00000800) {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Attribute 0x00000800");
        captainslog_warn((const char *)str);
    }

    if (header->Attributes & W3D_MESH_FLAG_PRELIT_MASK) {
        if (!header->PrelitVersion) {
            StringClass str;
            str.Format("W3D_CHUNK_MESH_HEADER3 Unknown value of Attribute PrelitVersion: %x", header->PrelitVersion);
            captainslog_warn((const char *)str);
        }
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 N/A PrelitVersion");
        captainslog_warn((const char *)str);
    }

    if (header->VertexChannels & 0xFFFFFF00) {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Vertex Channels %x", header->VertexChannels);
        captainslog_warn((const char *)str);
    }

    if (header->FaceChannels & 0xFFFFFFFE) {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Face Channels %x", header->FaceChannels);
        captainslog_warn((const char *)str);
    }

    WriteChunkRaw(csave, W3D_CHUNK_MESH_HEADER3, header, sizeof(W3dMeshHeader3Struct));
}

READ_WRITE_CHUNK_STRING(W3D_CHUNK_MESH_USER_TEXT)

READ_WRITE_CHUNK(W3D_CHUNK_NEAR_ATTENUATION, W3dLightAttenuationStruct)

READ_WRITE_CHUNK(W3D_CHUNK_NODE, W3dHModelNodeStruct)

READ_WRITE_CHUNK(W3D_CHUNK_NULL_OBJECT, W3dNullObjectStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_PER_FACE_TEXCOORD_IDS, Vector3i)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_PER_TRI_MATERIALS, uint16_t)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_PIVOT_FIXUPS, W3dPivotFixupStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_PIVOTS, W3dPivotStruct)

READ_WRITE_CHUNK(W3D_CHUNK_PLACEHOLDER, W3dPlaceholderStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_POINTS, W3dVectorStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_PRELIT_UNLIT)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_PRELIT_VERTEX)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_PS2_SHADERS, W3dPS2ShaderStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SCG, W3dRGBStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SHADER_IDS, uint32_t)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SHADERS, W3dShaderStruct)

READ_WRITE_CHUNK(W3D_CHUNK_SKIN_NODE, W3dHModelNodeStruct)

READ_WRITE_CHUNK(W3D_CHUNK_SPOT_LIGHT_INFO, W3dSpotLightStruct)

READ_WRITE_CHUNK(W3D_CHUNK_SPOT_LIGHT_INFO_5_0, W3dSpotLightStruct_v5_0)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_STAGE_TEXCOORDS, W3dTexCoordStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SURRENDER_NORMALS, W3dVectorStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_TEXCOORDS, W3dTexCoordStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_TEXTURE)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_TEXTURE_IDS, uint32_t)

void Read_W3D_CHUNK_TEXTURE_INFO(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dTextureInfoStruct), "Chunk W3D_CHUNK_TEXTURE_INFO size does not match data size");
    W3dTextureInfoStruct *info = (W3dTextureInfoStruct *)rawData;

    int hint = info->Attributes & 0xF00;

    if (hint != W3DTEXTURE_HINT_BASE && hint != W3DTEXTURE_HINT_EMISSIVE && hint != W3DTEXTURE_HINT_ENVIRONMENT
        && hint != W3DTEXTURE_HINT_SHINY_MASK) {
        StringClass str;
        str.Format("W3D_CHUNK_TEXTURE_INFO Unknown Hints %x", hint);
        captainslog_warn((const char *)str);
    }

    if (info->Attributes & 0xE000) {
        StringClass str;
        str.Format("W3D_CHUNK_TEXTURE_INFO Unknown Flags %x", info->Attributes & 0xE000);
        captainslog_warn((const char *)str);
    }

    if (info->AnimType != W3DTEXTURE_ANIM_LOOP && info->AnimType != W3DTEXTURE_ANIM_PINGPONG
        && info->AnimType != W3DTEXTURE_ANIM_ONCE && info->AnimType != W3DTEXTURE_ANIM_MANUAL) {
        StringClass str;
        str.Format("W3D_CHUNK_TEXTURE_INFO Unknown Anim Type %x", info->AnimType);
        captainslog_warn((const char *)str);
    }

    AddData(data, "W3D_CHUNK_TEXTURE_INFO", "W3dTextureInfoStruct", "", (void *)info);
}

void Write_W3D_CHUNK_TEXTURE_INFO(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dTextureInfoStruct *info = (W3dTextureInfoStruct *)data->data->info->value;
    int hint = info->Attributes & 0xF00;

    if (hint != W3DTEXTURE_HINT_BASE && hint != W3DTEXTURE_HINT_EMISSIVE && hint != W3DTEXTURE_HINT_ENVIRONMENT
        && hint != W3DTEXTURE_HINT_SHINY_MASK) {
        StringClass str;
        str.Format("W3D_CHUNK_TEXTURE_INFO Unknown Hints %x", hint);
        captainslog_warn((const char *)str);
    }

    if (info->Attributes & 0xE000) {
        StringClass str;
        str.Format("W3D_CHUNK_TEXTURE_INFO Unknown Flags %x", info->Attributes & 0xE000);
        captainslog_warn((const char *)str);
    }

    if (info->AnimType != W3DTEXTURE_ANIM_LOOP && info->AnimType != W3DTEXTURE_ANIM_PINGPONG
        && info->AnimType != W3DTEXTURE_ANIM_ONCE && info->AnimType != W3DTEXTURE_ANIM_MANUAL) {
        StringClass str;
        str.Format("W3D_CHUNK_TEXTURE_INFO Unknown Anim Type %x", info->AnimType);
        captainslog_warn((const char *)str);
    }

    WriteChunkRaw(csave, W3D_CHUNK_TEXTURE_INFO, info, sizeof(W3dTextureInfoStruct));
}

READ_WRITE_CHUNK_STRING(W3D_CHUNK_TEXTURE_NAME)

void Read_W3D_CHUNK_TEXTURE_REPLACER_INFO(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    W3dTextureReplacerHeaderStruct *header = (W3dTextureReplacerHeaderStruct *)rawData;
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dTextureReplacerHeaderStruct)
                + header->ReplacedTexturesCount * sizeof(W3dTextureReplacerStruct),
        "Chunk W3D_CHUNK_TEXTURE_REPLACER_INFO size does not match data size");

    AddData(data, "W3D_CHUNK_TEXTURE_REPLACER_INFO", "W3dTextureReplacerHeaderStruct[][]", "", (void *)rawData);
}

void Write_W3D_CHUNK_TEXTURE_REPLACER_INFO(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    char *rawData = (char *)data->data->info->value;
    WriteChunkRaw(csave, W3D_CHUNK_TEXTURE_REPLACER_INFO, rawData, data->data->chunkSize);
}

READ_WRITE_SUBCHUNKS(W3D_CHUNK_TEXTURE_STAGE)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_TEXTURES)

READ_WRITE_CHUNK(W3D_CHUNK_TRANSFORM_NODE, W3dPlaceholderStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_TRIANGLES, W3dTriStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_VERTEX_COLORS, W3dRGBStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_VERTEX_INFLUENCES, W3dVertInfStruct)

READ_WRITE_CHUNK_STRING(W3D_CHUNK_VERTEX_MAPPER_ARGS0)

READ_WRITE_CHUNK_STRING(W3D_CHUNK_VERTEX_MAPPER_ARGS1)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_VERTEX_MATERIAL)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_VERTEX_MATERIAL_IDS, uint32_t)

void Read_W3D_CHUNK_VERTEX_MATERIAL_INFO(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dVertexMaterialStruct), "Chunk W3D_CHUNK_VERTEX_MATERIAL_INFO size does not match data size");
    W3dVertexMaterialStruct *material = (W3dVertexMaterialStruct *)rawData;

    if (material->Attributes & 0x00000010) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Attribute 0x00000010");
        captainslog_warn((const char *)str);
    }

    if (material->Attributes & 0x00000020) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Attribute 0x00000020");
        captainslog_warn((const char *)str);
    }

    if (material->Attributes & 0x00000040) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Attribute 0x00000040");
        captainslog_warn((const char *)str);
    }

    if (material->Attributes & 0x00000080) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Attribute 0x00000080");
        captainslog_warn((const char *)str);
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) > W3DVERTMAT_STAGE0_MAPPING_GRID_WS_ENVIRONMENT) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Stage 0 Mapper %x",
            material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK);
        captainslog_warn((const char *)str);
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) > W3DVERTMAT_STAGE1_MAPPING_GRID_WS_ENVIRONMENT) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Stage 1 Mapper %x",
            material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK);
        captainslog_warn((const char *)str);
    }

    if (material->Attributes & W3DVERTMAT_PSX_MASK) {
        if (material->Attributes & 0xF0000000) {
            StringClass str;
            str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown PSX material flag %x", material->Attributes & 0xF0000000);
            captainslog_warn((const char *)str);
        }
    }
    AddData(data, "W3D_CHUNK_VERTEX_MATERIAL_INFO", "W3dVertexMaterialStruct", "", (void *)material);
}

void Write_W3D_CHUNK_VERTEX_MATERIAL_INFO(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dVertexMaterialStruct *material = (W3dVertexMaterialStruct *)data->data->info->value;
    if (material->Attributes & 0x00000010) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Attribute 0x00000010");
        captainslog_warn((const char *)str);
    }

    if (material->Attributes & 0x00000020) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Attribute 0x00000020");
        captainslog_warn((const char *)str);
    }

    if (material->Attributes & 0x00000040) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Attribute 0x00000040");
        captainslog_warn((const char *)str);
    }

    if (material->Attributes & 0x00000080) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Attribute 0x00000080");
        captainslog_warn((const char *)str);
    }

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) > W3DVERTMAT_STAGE0_MAPPING_GRID_WS_ENVIRONMENT) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Stage 0 Mapper %x",
            material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK);
        captainslog_warn((const char *)str);
    }

    if ((material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) > W3DVERTMAT_STAGE1_MAPPING_GRID_WS_ENVIRONMENT) {
        StringClass str;
        str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown Stage 1 Mapper %x",
            material->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK);
        captainslog_warn((const char *)str);
    }

    if (material->Attributes & W3DVERTMAT_PSX_MASK) {
        if (material->Attributes & 0xF0000000) {
            StringClass str;
            str.Format("W3D_CHUNK_VERTEX_MATERIAL_INFO Unknown PSX material flag %x", material->Attributes & 0xF0000000);
            captainslog_warn((const char *)str);
        }
    }

    WriteChunkRaw(csave, W3D_CHUNK_VERTEX_MATERIAL_INFO, material, sizeof(W3dVertexMaterialStruct));
}

READ_WRITE_CHUNK_STRING(W3D_CHUNK_VERTEX_MATERIAL_NAME)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_VERTEX_MATERIALS)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_VERTEX_NORMALS, W3dVectorStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_VERTEX_SHADE_INDICES, uint32_t)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_VERTICES, W3dVectorStruct)

void Read_W3D_CHUNK_EMITTER_LINE_PROPERTIES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dEmitterLinePropertiesStruct), "Chunk W3D_CHUNK_EMITTER_LINE_PROPERTIES size does not match data size");
    W3dEmitterLinePropertiesStruct *props = (W3dEmitterLinePropertiesStruct *)rawData;

    if (props->Flags & 0x00FFFFF0) {
        StringClass str;
        str.Format("W3D_CHUNK_EMITTER_LINE_PROPERTIES Unknown Emitter Line Properties flags %x", props->Flags & 0x00FFFFF0);
        captainslog_warn((const char *)str);
    }

    size_t mapmode = props->Flags >> W3D_ELINE_TEXTURE_MAP_MODE_OFFSET;
    if (mapmode != W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP && mapmode != W3D_ELINE_UNIFORM_LENGTH_TEXTURE_MAP
        && mapmode != W3D_ELINE_TILED_TEXTURE_MAP) {
        StringClass str;
        str.Format("W3D_CHUNK_EMITTER_LINE_PROPERTIES Unknown Emitter Mapping Mode %x", mapmode);
        captainslog_warn((const char *)str);
    }

    AddData(data, "W3D_CHUNK_EMITTER_LINE_PROPERTIES", "W3dEmitterLinePropertiesStruct", "", (void *)props);
}

void Write_W3D_CHUNK_EMITTER_LINE_PROPERTIES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dEmitterLinePropertiesStruct *props = (W3dEmitterLinePropertiesStruct *)data->data->info->value;
    if (props->Flags & 0x00FFFFF0) {
        StringClass str;
        str.Format("W3D_CHUNK_EMITTER_LINE_PROPERTIES Unknown Emitter Line Properties flags %x", props->Flags & 0x00FFFFF0);
        captainslog_warn((const char *)str);
    }

    size_t mapmode = props->Flags >> W3D_ELINE_TEXTURE_MAP_MODE_OFFSET;
    if (mapmode != W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP && mapmode != W3D_ELINE_UNIFORM_LENGTH_TEXTURE_MAP
        && mapmode != W3D_ELINE_TILED_TEXTURE_MAP) {
        StringClass str;
        str.Format("W3D_CHUNK_EMITTER_LINE_PROPERTIES Unknown Emitter Mapping Mode %x", mapmode);
        captainslog_warn((const char *)str);
    }

    WriteChunkRaw(csave, W3D_CHUNK_EMITTER_LINE_PROPERTIES, props, sizeof(W3dEmitterLinePropertiesStruct));
}

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SECONDARY_VERTICES, W3dVectorStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SECONDARY_VERTEX_NORMALS, W3dVectorStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_TANGENTS, W3dVectorStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_BINORMALS, W3dVectorStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_COMPRESSED_ANIMATION)

int flavor = ANIM_FLAVOR_TIMECODED;

void Read_W3D_CHUNK_COMPRESSED_ANIMATION_HEADER(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dCompressedAnimHeaderStruct), "Chunk W3D_CHUNK_COMPRESSED_ANIMATION_HEADER size does not match data size");
    W3dCompressedAnimHeaderStruct *header = (W3dCompressedAnimHeaderStruct *)rawData;

    if (header->Flavor >= ANIM_FLAVOR_VALID) {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_HEADER Unknown Flavor Type %x", header->Flavor);
        captainslog_warn((const char *)str);
    }

    flavor = header->Flavor;
    AddData(data, "W3D_CHUNK_COMPRESSED_ANIMATION_HEADER", "W3dCompressedAnimHeaderStruct", "", (void *)header);
}

void Write_W3D_CHUNK_COMPRESSED_ANIMATION_HEADER(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dCompressedAnimHeaderStruct *header = (W3dCompressedAnimHeaderStruct *)data->data->info->value;

    if (header->Flavor >= ANIM_FLAVOR_VALID) {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_HEADER Unknown Flavor Type %x", header->Flavor);
        captainslog_warn((const char *)str);
    }

    flavor = header->Flavor;
    WriteChunkRaw(csave, W3D_CHUNK_COMPRESSED_ANIMATION_HEADER, header, sizeof(W3dCompressedAnimHeaderStruct));
}

void Read_W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);

    if (flavor == ANIM_FLAVOR_TIMECODED) {
        W3dTimeCodedAnimChannelStruct *channel = (W3dTimeCodedAnimChannelStruct *)rawData;
        captainslog_dbgassert(data->data->chunkSize == sizeof(W3dTimeCodedAnimChannelStruct)
                    + (((cload.Cur_Chunk_Length() - sizeof(W3dTimeCodedAnimChannelStruct)) >> 2) + 1) * sizeof(int),
            "Chunk W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL size does not match data size");

        if (channel->Flags <= ANIM_CHANNEL_VIS) {
        } else {
            StringClass str;
            str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
            captainslog_warn((const char *)str);
        }
        AddData(data, "W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL", "W3dTimeCodedAnimChannelStruct", "", (void *)channel);
        } else {
        W3dAdaptiveDeltaAnimChannelStruct *channel = (W3dAdaptiveDeltaAnimChannelStruct *)rawData;
        captainslog_dbgassert(data->data->chunkSize == sizeof(W3dAdaptiveDeltaAnimChannelStruct)
                    + (((cload.Cur_Chunk_Length() - sizeof(W3dAdaptiveDeltaAnimChannelStruct)) >> 2) + 1) * sizeof(int),
            "Chunk W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL size does not match data size");

        if (channel->Flags <= ANIM_CHANNEL_VIS) {
        } else {
            StringClass str;
            str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
            captainslog_warn((const char *)str);
        }

        AddData(data, "W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL", "W3dAdaptiveDeltaAnimChannelStruct", "", (void *)channel);
        }
}

void Write_W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    char *rawData = (char *)data->data->info->value;
    // TODO valid data
    WriteChunkRaw(csave, W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL, rawData, data->data->chunkSize);
}

void Read_W3D_CHUNK_COMPRESSED_BIT_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    W3dTimeCodedBitChannelStruct *channel = (W3dTimeCodedBitChannelStruct *)rawData;
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dTimeCodedBitChannelStruct) + channel->NumTimeCodes * sizeof(int),
        "Chunk W3D_CHUNK_COMPRESSED_BIT_CHANNEL size does not match data size");

    if (channel->Flags > BIT_CHANNEL_TIMECODED_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_BIT_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    AddData(data, "W3D_CHUNK_COMPRESSED_BIT_CHANNEL", "W3dTimeCodedBitChannelStruct", "", (void *)channel);

}

void Write_W3D_CHUNK_COMPRESSED_BIT_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dTimeCodedBitChannelStruct *channel = (W3dTimeCodedBitChannelStruct *)data->data->info->value;
    if (channel->Flags > BIT_CHANNEL_TIMECODED_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_BIT_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    WriteChunkRaw(csave, W3D_CHUNK_COMPRESSED_BIT_CHANNEL, channel, data->data->chunkSize);
}

READ_WRITE_SUBCHUNKS(W3D_CHUNK_MORPH_ANIMATION)

READ_WRITE_CHUNK(W3D_CHUNK_MORPHANIM_HEADER, W3dMorphAnimHeaderStruct)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_MORPHANIM_CHANNEL)

READ_WRITE_CHUNK_STRING(W3D_CHUNK_MORPHANIM_POSENAME)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_MORPHANIM_KEYDATA, W3dMorphAnimKeyStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_MORPHANIM_PIVOTCHANNELDATA, uint32_t)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_SOUNDROBJ)

READ_WRITE_CHUNK(W3D_CHUNK_SOUNDROBJ_HEADER, W3dSoundRObjHeaderStruct)

#define READ_FLOAT(id, name) \
    case id: { \
        float f; \
        cload.Read(&f, sizeof(f)); \
        AddFloat(data, #name, f); \
        break; \
    }

#define READ_INT(id, name) \
    case id: { \
        int i; \
        cload.Read(&i, sizeof(i)); \
        AddInt32(data, #name, i); \
        break; \
    }

#define READ_BOOL(id, name) \
    case id: { \
        bool b; \
        cload.Read(&b, sizeof(b)); \
        AddInt8(data, #name, b); \
        break; \
    }

#define READ_VECTOR(id, name) \
    case id: { \
        Vector3 v; \
        cload.Read(&v, sizeof(v)); \
        W3dVectorStruct v2; \
        v2.x = v.X; \
        v2.y = v.Y; \
        v2.z = v.Z; \
        AddVector(data, #name, &v2); \
        break; \
    }

#define READ_STRING(id, name) \
    case id: { \
        StringClass str; \
        cload.Read(str.Get_Buffer(cload.Cur_Micro_Chunk_Length()), cload.Cur_Micro_Chunk_Length()); \
        AddString(data, #name, str, "String"); \
        break; \
    }

void Read_W3D_CHUNK_SOUNDROBJ_DEFINITION(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    // TODO: Implement Read_W3D_CHUNK_SOUNDROBJ_DEFINITION
    captainslog_warn("Read_W3D_CHUNK_SOUNDROBJ_DEFINITION not implemented");
    while (cload.Open_Chunk()) {
        switch (cload.Cur_Chunk_ID()) {
            case 0x100:
                while (cload.Open_Micro_Chunk()) {
                    switch (cload.Cur_Micro_Chunk_ID()) {
                        READ_FLOAT(3, m_Priority);
                        READ_FLOAT(4, m_Volume);
                        READ_FLOAT(5, m_Pan);
                        READ_INT(6, m_LoopCount);
                        READ_FLOAT(7, m_DropoffRadius);
                        READ_FLOAT(8, m_MaxVolRadius);
                        READ_INT(9, m_Type);
                        READ_BOOL(10, m_Is3DSound);
                        READ_STRING(11, m_Filename);
                        READ_STRING(12, m_DisplayText);
                        READ_FLOAT(18, m_StartOffset);
                        READ_FLOAT(19, m_PitchFactor);
                        READ_FLOAT(20, m_PitchFactorRandomizer);
                        READ_FLOAT(21, m_VolumeRandomizer);
                        READ_INT(22, m_VirtualChannel);
                        READ_INT(13, m_LogicalType);
                        READ_FLOAT(14, m_LogicalNotifDelay);
                        READ_BOOL(15, m_CreateLogicalSound);
                        READ_FLOAT(16, m_LogicalDropoffRadius);
                        READ_VECTOR(17, m_SphereColor);
                    }
                    cload.Close_Micro_Chunk();
                }
                break;
            case 0x200:
                while (cload.Open_Chunk()) {
                    if (cload.Cur_Chunk_ID() == 0x100) {
                        while (cload.Open_Micro_Chunk()) {
                            switch (cload.Cur_Micro_Chunk_ID()) {
                                READ_INT(1, m_ID);
                                READ_STRING(3, m_Name);
                            }
                            cload.Close_Micro_Chunk();
                        }
                    }
                    cload.Close_Chunk();
                }
                break;
        }
        cload.Close_Chunk();
    }
}

void Write_W3D_CHUNK_SOUNDROBJ_DEFINITION(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Write_W3D_CHUNK_SOUNDROBJ_DEFINITION
    captainslog_warn("Write_W3D_CHUNK_SOUNDROBJ_DEFINITION not implemented");
}

void DoVector3Channel(ChunkLoadClass &cload, ChunkTreePtr &data, const char *name)
{
    int i = 0;

    for (; cload.Open_Chunk(); cload.Close_Chunk()) {
        if (cload.Cur_Chunk_ID() == 51709961) { // PrimitiveAnimationChannelClass
            for (; cload.Open_Micro_Chunk(); cload.Close_Micro_Chunk()) {
                if (cload.Cur_Micro_Chunk_ID() == 1) {
                    Vector3 value;
                    float time;
                    cload.Read(&value, sizeof(value));
                    cload.Read(&time, sizeof(time));
                    StringClass str;
                    str.Format("%s[%d].Value.X", name, i);
                    AddFloat(data, str, value.X);
                    str.Format("%s[%d].Value.Y", name, i);
                    AddFloat(data, str, value.Y);
                    str.Format("%s[%d].Value.Z", name, i);
                    AddFloat(data, str, value.Z);
                    str.Format("%s[%d].time", name, i);
                    AddFloat(data, str, time);
                    i++;
                }
            }
        }
    }
}

void DoVector2Channel(ChunkLoadClass &cload, ChunkTreePtr &data, const char *name)
{
    int i = 0;

    for (; cload.Open_Chunk(); cload.Close_Chunk()) {
        if (cload.Cur_Chunk_ID() == 51709961) { // PrimitiveAnimationChannelClass
            for (; cload.Open_Micro_Chunk(); cload.Close_Micro_Chunk()) {
                if (cload.Cur_Micro_Chunk_ID() == 1) {
                    Vector2 value;
                    float time;
                    cload.Read(&value, sizeof(value));
                    cload.Read(&time, sizeof(time));
                    StringClass str;
                    str.Format("%s[%d].Value.X", name, i);
                    AddFloat(data, str, value.X);
                    str.Format("%s[%d].Value.Y", name, i);
                    AddFloat(data, str, value.Y);
                    str.Format("%s[%d].time", name, i);
                    AddFloat(data, str, time);
                    i++;
                }
            }
        }
    }
}

void DofloatChannel(ChunkLoadClass &cload, ChunkTreePtr &data, const char *name)
{
    int i = 0;

    for (; cload.Open_Chunk(); cload.Close_Chunk()) {
        if (cload.Cur_Chunk_ID() == 51709961) { // PrimitiveAnimationChannelClass
            for (; cload.Open_Micro_Chunk(); cload.Close_Micro_Chunk()) {
                if (cload.Cur_Micro_Chunk_ID() == 1) {
                    float value;
                    float time;
                    cload.Read(&value, sizeof(value));
                    cload.Read(&time, sizeof(time));
                    StringClass str;
                    str.Format("%s[%d].Value", name, i);
                    AddFloat(data, str, value);
                    str.Format("%s[%d].time", name, i);
                    AddFloat(data, str, time);
                    i++;
                }
            }
        }
    }
}

void DoAlphaVectorStructChannel(ChunkLoadClass &cload, ChunkTreePtr &data, const char *name)
{
    int i = 0;

    for (; cload.Open_Chunk(); cload.Close_Chunk()) {
        if (cload.Cur_Chunk_ID() == 51709961) { // PrimitiveAnimationChannelClass
            for (; cload.Open_Micro_Chunk(); cload.Close_Micro_Chunk()) {
                if (cload.Cur_Micro_Chunk_ID() == 1) {
                    AlphaVectorStruct value;
                    float time;
                    cload.Read(&value, sizeof(value));
                    cload.Read(&time, sizeof(time));
                    StringClass str;
                    str.Format("%s[%d].Value.Quat.X", name, i);
                    AddFloat(data, str, value.Quat.X);
                    str.Format("%s[%d].Value.Quat.Y", name, i);
                    AddFloat(data, str, value.Quat.Y);
                    str.Format("%s[%d].Value.Quat.Z", name, i);
                    AddFloat(data, str, value.Quat.Z);
                    str.Format("%s[%d].Value.Quat.W", name, i);
                    AddFloat(data, str, value.Quat.W);
                    str.Format("%s[%d].Value.Magnitude", name, i);
                    AddFloat(data, str, value.Magnitude);
                    str.Format("%s[%d].time", name, i);
                    AddFloat(data, str, time);
                    i++;
                }
            }
        }
    }
}

void Read_W3D_CHUNK_RING(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    // TODO: Implement Read_W3D_CHUNK_RING
    captainslog_warn("Read_W3D_CHUNK_RING not implemented");
    while (cload.Open_Chunk()) {
        switch (cload.Cur_Chunk_ID()) {
            case 1: {
                W3dRingStruct *RingStruct;
                cload.Read(RingStruct, sizeof(RingStruct));

                if (RingStruct->Flags & 0xFFFFFFFC) {
                    StringClass str;
                    str.Format("W3D_CHUNK_RING Unknown Ring Flags %x", RingStruct->Flags & 0xFFFFFFFC);
                    captainslog_warn((const char *)str);
                }

                AddData(data, "W3D_CHUNK_RING", "W3dRingStruct", "", (void *)RingStruct);
                break;
            }
            case 2:
                DoVector3Channel(cload, data, "ColorChannel");
                break;
            case 3:
                DofloatChannel(cload, data, "AlphaChannel");
                break;
            case 4:
                DoVector2Channel(cload, data, "InnerScaleChannel");
                break;
            case 5:
                DoVector2Channel(cload, data, "OuterScaleChannel");
                break;
        }
        cload.Close_Chunk();
    }
}

void Write_W3D_CHUNK_RING(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Write_W3D_CHUNK_RING
    captainslog_warn("Write_W3D_CHUNK_RING not implemented");
}

void Read_W3D_CHUNK_SPHERE(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    // TODO: Implement Read_W3D_CHUNK_SPHERE
    captainslog_warn("Read_W3D_CHUNK_SPHERE not implemented");
    while (cload.Open_Chunk()) {
        switch (cload.Cur_Chunk_ID()) {
            case 1: {
                W3dSphereStruct *SphereStruct;
                cload.Read(SphereStruct, sizeof(SphereStruct));

                if (SphereStruct->Flags & 0xFFFFFFF0) {
                    StringClass str;
                    str.Format("W3D_CHUNK_SPHERE Unknown Sphere Flags %x", SphereStruct->Flags & 0xFFFFFFFC);
                    captainslog_warn((const char *)str);
                }
                AddData(data, "W3D_CHUNK_SPHERE", "W3dSphereStruct", "", (void *)SphereStruct);

            } break;
            case 2:
                DoVector3Channel(cload, data, "ColorChannel");
                break;
            case 3:
                DofloatChannel(cload, data, "AlphaChannel");
                break;
            case 4:
                DoVector3Channel(cload, data, "ScaleChannel");
                break;
            case 5:
                DoAlphaVectorStructChannel(cload, data, "VectorChannel");
                break;
        }
        cload.Close_Chunk();
    }
}

void Write_W3D_CHUNK_SPHERE(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Write_W3D_CHUNK_SPHERE
    captainslog_warn("Write_W3D_CHUNK_SPHERE not implemented");
}

READ_WRITE_SUBCHUNKS(W3D_CHUNK_SHDMESH)

READ_WRITE_CHUNK_STRING(W3D_CHUNK_SHDMESH_NAME)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_SHDSUBMESH)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_SHDSUBMESH_SHADER)

READ_WRITE_CHUNK(W3D_CHUNK_SHDSUBMESH_SHADER_TYPE, uint32_t)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_VERTICES, W3dVectorStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_VERTEX_NORMALS, W3dVectorStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_TRIANGLES, Vector3i16)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_VERTEX_SHADE_INDICES, uint32_t)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_UV0, W3dTexCoordStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_UV1, W3dTexCoordStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_S, W3dVectorStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_T, W3dVectorStruct)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_SXT, W3dVectorStruct)

READ_WRITE_CHUNK(W3D_CHUNK_EMITTER_EXTRA_INFO, W3dEmitterExtraInfoStruct)

READ_WRITE_CHUNK_STRING(W3D_CHUNK_SHDMESH_USER_TEXT)

READ_WRITE_CHUNK_ARRAY(W3D_CHUNK_FXSHADER_IDS, uint32_t)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_FX_SHADERS)

READ_WRITE_SUBCHUNKS(W3D_CHUNK_FX_SHADER)

void Read_W3D_CHUNK_FX_SHADER_INFO(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    // TODO Fix W3dFXShaderStruct
    captainslog_warn("Read_W3D_CHUNK_FX_SHADER_INFO not implemented");
    char *rawData = ReadChunkRaw(cload);
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dFXShaderStruct) + 1, "Chunk W3D_CHUNK_FX_SHADER_INFO size does not match data size");
    uint8_t *version = (uint8_t *)rawData;
    AddInt8(data, "Version", *version);
    W3dFXShaderStruct *shader = (W3dFXShaderStruct *)(rawData + 1);
    AddString(data, "ShaderName", shader->shadername, "string");
    AddInt8(data, "Technique", shader->technique);
    delete[] rawData;
}

void Write_W3D_CHUNK_FX_SHADER_INFO(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Write_W3D_CHUNK_FX_SHADER_INFO
    captainslog_warn("Write_W3D_CHUNK_FX_SHADER_INFO not implemented");
//    char *rawData = new char[sizeof(W3dFXShaderStruct) + 1];
}

struct W3dFXShaderConstantStruct
{
    uint32_t Type;
    uint32_t Length;
    std::string ConstantName;
    union
    {
        std::string Texture;
        std::vector<float> Floats;
        uint32_t Int;
        bool Bool;
    } Value;

    W3dFXShaderConstantStruct &operator=(const char *rawData)
    {
        Type = *(uint32_t *)rawData;
        Length = *(uint32_t *)(rawData + 4);
        ConstantName = std::string(rawData + 8, Length);

        if (Type == CONSTANT_TYPE_TEXTURE) {
            Value.Texture = std::string(rawData + 8 + Length);
        } else if (Type >= CONSTANT_TYPE_FLOAT1 && Type <= CONSTANT_TYPE_FLOAT4) {
            int count = Type - 1;
            float *floats = (float *)(rawData + 8 + Length);
            Value.Floats.assign(floats, floats + count);
        } else if (Type == CONSTANT_TYPE_INT) {
            Value.Int = *(uint32_t *)(rawData + 8 + Length);
        } else if (Type == CONSTANT_TYPE_BOOL) {
            Value.Bool = *(uint8_t *)(rawData + 8 + Length) != 0;
        } else {
            // Handle unknown types
            Value.Texture = "Unknown";
        }

        return *this;
    }
};

void Read_W3D_CHUNK_FX_SHADER_CONSTANT(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    // TODO check valid
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dFXShaderConstantStruct), "Chunk W3D_CHUNK_FX_SHADER_CONSTANT size does not match data size");
    W3dFXShaderConstantStruct *constant = (W3dFXShaderConstantStruct *)rawData;
    uint32_t type = *(uint32_t *)rawData;
    uint32_t length = *(uint32_t *)(rawData + 4);
    uint32_t expectedSize = 4 + 4 + length; // 4 bytes for 'type' and 'length' plus length for constant name
    if (type == CONSTANT_TYPE_TEXTURE) {
        expectedSize += 4 + strlen((char *)(rawData + expectedSize)); // add texture size
    } else if (type >= CONSTANT_TYPE_FLOAT1 && type <= CONSTANT_TYPE_FLOAT4) {
        expectedSize += sizeof(float) * (type - 1); // add float array size
    } else if (type == CONSTANT_TYPE_INT) {
        expectedSize += sizeof(uint32_t); // add int size
    } else if (type == CONSTANT_TYPE_BOOL) {
        expectedSize += sizeof(uint8_t); // add bool size
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_FX_SHADER_CONSTANT Unknown Constant Type %x", constant->Type);
        captainslog_warn((const char *)str);
    }

    captainslog_dbgassert(data->data->chunkSize == expectedSize,
        "Chunk W3D_CHUNK_FX_SHADER_CONSTANT size does not match expected size");


    if (constant->Value.Texture == "Unknown") {
    }
    AddData(data, "W3D_CHUNK_FX_SHADER_CONSTANT", "W3dFXShaderConstantStruct", "", (void *)constant);
}

void Write_W3D_CHUNK_FX_SHADER_CONSTANT(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dFXShaderConstantStruct *constant = (W3dFXShaderConstantStruct *)data->data->info->value;
    if (constant->Value.Texture == "Unknown") {
        StringClass str;
        str.Format("W3D_CHUNK_FX_SHADER_CONSTANT Unknown Constant Type %x", constant->Type);
        captainslog_warn((const char *)str);
    }
    WriteChunkRaw(csave, W3D_CHUNK_FX_SHADER_CONSTANT, constant, sizeof(W3dFXShaderConstantStruct));
}

struct W3dCompressedMotionChannelStructNew
{
    uint8_t Zero;
    uint8_t Flavor;
    uint8_t VectorLen;
    uint8_t Flags;
    uint16_t NumTimeCodes;
    uint16_t Pivot;

    std::vector<uint16_t> KeyFrames; // ANIM_FLAVOR_NEW_TIMECODED
    float Scale; // עבור Scale
    std::vector<float> Initial;
    std::vector<uint32_t> Data;

    W3dCompressedMotionChannelStructNew &operator=(const char *rawData)
    {
        Zero = *(uint8_t *)rawData;
        Flavor = *(uint8_t *)(rawData + 1);
        VectorLen = *(uint8_t *)(rawData + 2);
        Flags = *(uint8_t *)(rawData + 3);
        NumTimeCodes = *(uint16_t *)(rawData + 4);
        Pivot = *(uint16_t *)(rawData + 6);

        size_t offset = sizeof(W3dCompressedMotionChannelStructNew);

        if (Flavor == ANIM_FLAVOR_NEW_TIMECODED) {
            KeyFrames.assign(
                (uint16_t *)(rawData + offset), (uint16_t *)(rawData + offset + NumTimeCodes * sizeof(uint16_t)));
            offset += NumTimeCodes * sizeof(uint16_t);

            int datalen = VectorLen * NumTimeCodes;
            if (NumTimeCodes & 1) {
                offset += 2; // Padding for odd NumTimeCodes
            }
            Data.assign((uint32_t *)(rawData + offset), (uint32_t *)(rawData + offset + datalen * sizeof(uint32_t)));
        } else {
            Scale = *(float *)(rawData + offset);
            offset += sizeof(float);

            Initial.assign((float *)(rawData + offset), (float *)(rawData + offset + VectorLen * sizeof(float)));
            offset += VectorLen * sizeof(float);

            int count = (NumTimeCodes * VectorLen - sizeof(W3dCompressedMotionChannelStructNew) - sizeof(float)
                            - sizeof(float) * VectorLen) / sizeof(uint32_t);
            Data.assign((uint32_t *)(rawData + offset), (uint32_t *)(rawData + offset + count * sizeof(uint32_t)));
        }
        return *this;
    }
};

void Read_W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *rawData = ReadChunkRaw(cload);
    captainslog_dbgassert(data->data->chunkSize == sizeof(W3dCompressedMotionChannelStructNew), "Chunk W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL size does not match data size");
    W3dCompressedMotionChannelStructNew *channel = (W3dCompressedMotionChannelStructNew *)rawData;
    if (channel->Flavor >= ANIM_FLAVOR_NEW_VALID) {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL Unknown Flavor Type %x", channel->Flavor);
        captainslog_warn((const char *)str);
    }

    if (channel->Flags > ANIM_CHANNEL_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }

    AddData(
        data, "W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL", "W3dCompressedMotionChannelStructNew", "", (void *)channel);
}

void Write_W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dCompressedMotionChannelStructNew *channel = (W3dCompressedMotionChannelStructNew *)data->data->info->value;
    if (channel->Flavor >= ANIM_FLAVOR_NEW_VALID) {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL Unknown Flavor Type %x", channel->Flavor);
        captainslog_warn((const char *)str);
    }

    if (channel->Flags > ANIM_CHANNEL_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    WriteChunkRaw(
        csave, W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL, channel, sizeof(W3dCompressedMotionChannelStructNew));
}

std::map<int, ChunkIOFuncs> chunkFuncMap;

#define CHUNK(id) \
    { \
        ChunkIOFuncs c; \
        c.name = #id; \
        c.ReadChunk = Read_##id; \
        c.WriteChunk = Write_##id; \
        chunkFuncMap[id] = c; \
    }

void ChunkManager::InitiateChunkFuncMap()
{
    CHUNK(O_W3D_CHUNK_MATERIALS)
    CHUNK(O_W3D_CHUNK_MATERIALS2)
    CHUNK(O_W3D_CHUNK_POV_QUADRANGLES)
    CHUNK(O_W3D_CHUNK_POV_TRIANGLES)
    CHUNK(O_W3D_CHUNK_QUADRANGLES)
    CHUNK(O_W3D_CHUNK_SURRENDER_TRIANGLES)
    CHUNK(O_W3D_CHUNK_TRIANGLES)
    CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_COLOR_KEYFRAME)
    CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME)
    CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_SIZE_KEYFRAME)
    CHUNK(OBSOLETE_W3D_CHUNK_SHADOW_NODE)
    CHUNK(W3D_CHUNK_AABTREE)
    CHUNK(W3D_CHUNK_AABTREE_HEADER)
    CHUNK(W3D_CHUNK_AABTREE_NODES)
    CHUNK(W3D_CHUNK_AABTREE_POLYINDICES)
    CHUNK(W3D_CHUNK_AGGREGATE)
    CHUNK(W3D_CHUNK_AGGREGATE_CLASS_INFO)
    CHUNK(W3D_CHUNK_AGGREGATE_HEADER)
    CHUNK(W3D_CHUNK_AGGREGATE_INFO)
    CHUNK(W3D_CHUNK_ANIMATION)
    CHUNK(W3D_CHUNK_ANIMATION_CHANNEL)
    CHUNK(W3D_CHUNK_ANIMATION_HEADER)
    CHUNK(W3D_CHUNK_BIT_CHANNEL)
    CHUNK(W3D_CHUNK_BOX)
    CHUNK(W3D_CHUNK_COLLECTION)
    CHUNK(W3D_CHUNK_COLLECTION_HEADER)
    CHUNK(W3D_CHUNK_COLLECTION_OBJ_NAME)
    CHUNK(W3D_CHUNK_COLLISION_NODE)
    CHUNK(W3D_CHUNK_DAMAGE)
    CHUNK(W3D_CHUNK_DAMAGE_COLORS)
    CHUNK(W3D_CHUNK_DAMAGE_HEADER)
    CHUNK(W3D_CHUNK_DAMAGE_VERTICES)
    CHUNK(W3D_CHUNK_DAZZLE)
    CHUNK(W3D_CHUNK_DAZZLE_NAME)
    CHUNK(W3D_CHUNK_DAZZLE_TYPENAME)
    CHUNK(W3D_CHUNK_DCG)
    CHUNK(W3D_CHUNK_DIG)
    CHUNK(W3D_CHUNK_EMITTER)
    CHUNK(W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES)
    CHUNK(W3D_CHUNK_EMITTER_FRAME_KEYFRAMES)
    CHUNK(W3D_CHUNK_EMITTER_HEADER)
    CHUNK(W3D_CHUNK_EMITTER_INFO)
    CHUNK(W3D_CHUNK_EMITTER_INFOV2)
    CHUNK(W3D_CHUNK_EMITTER_PROPS)
    CHUNK(W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES)
    CHUNK(W3D_CHUNK_EMITTER_USER_DATA)
    CHUNK(W3D_CHUNK_FAR_ATTENUATION)
    CHUNK(W3D_CHUNK_HIERARCHY)
    CHUNK(W3D_CHUNK_HIERARCHY_HEADER)
    CHUNK(W3D_CHUNK_HLOD)
    CHUNK(W3D_CHUNK_HLOD_AGGREGATE_ARRAY)
    CHUNK(W3D_CHUNK_HLOD_HEADER)
    CHUNK(W3D_CHUNK_HLOD_LOD_ARRAY)
    CHUNK(W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER)
    CHUNK(W3D_CHUNK_HLOD_PROXY_ARRAY)
    CHUNK(W3D_CHUNK_HLOD_SUB_OBJECT)
    CHUNK(W3D_CHUNK_HMODEL)
    CHUNK(OBSOLETE_W3D_CHUNK_HMODEL_AUX_DATA)
    CHUNK(W3D_CHUNK_HMODEL_HEADER)
    CHUNK(W3D_CHUNK_LIGHT)
    CHUNK(W3D_CHUNK_LIGHT_INFO)
    CHUNK(W3D_CHUNK_LIGHT_TRANSFORM)
    CHUNK(W3D_CHUNK_LIGHTSCAPE)
    CHUNK(W3D_CHUNK_LIGHTSCAPE_LIGHT)
    CHUNK(W3D_CHUNK_LOD)
    CHUNK(W3D_CHUNK_LODMODEL)
    CHUNK(W3D_CHUNK_LODMODEL_HEADER)
    CHUNK(W3D_CHUNK_MAP3_FILENAME)
    CHUNK(W3D_CHUNK_MAP3_INFO)
    CHUNK(W3D_CHUNK_MATERIAL_INFO)
    CHUNK(W3D_CHUNK_MATERIAL_PASS)
    CHUNK(W3D_CHUNK_MATERIAL3)
    CHUNK(W3D_CHUNK_MATERIAL3_DC_MAP)
    CHUNK(W3D_CHUNK_MATERIAL3_DI_MAP)
    CHUNK(W3D_CHUNK_MATERIAL3_INFO)
    CHUNK(W3D_CHUNK_MATERIAL3_NAME)
    CHUNK(W3D_CHUNK_MATERIAL3_SC_MAP)
    CHUNK(W3D_CHUNK_MATERIAL3_SI_MAP)
    CHUNK(W3D_CHUNK_MATERIALS3)
    CHUNK(W3D_CHUNK_MESH)
    CHUNK(W3D_CHUNK_MESH_HEADER)
    CHUNK(W3D_CHUNK_MESH_HEADER3)
    CHUNK(W3D_CHUNK_MESH_USER_TEXT)
    CHUNK(W3D_CHUNK_NEAR_ATTENUATION)
    CHUNK(W3D_CHUNK_NODE)
    CHUNK(W3D_CHUNK_NULL_OBJECT)
    CHUNK(W3D_CHUNK_PER_FACE_TEXCOORD_IDS)
    CHUNK(W3D_CHUNK_PER_TRI_MATERIALS)
    CHUNK(W3D_CHUNK_PIVOT_FIXUPS)
    CHUNK(W3D_CHUNK_PIVOTS)
    CHUNK(W3D_CHUNK_PLACEHOLDER)
    CHUNK(W3D_CHUNK_POINTS)
    CHUNK(W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS)
    CHUNK(W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE)
    CHUNK(W3D_CHUNK_PRELIT_UNLIT)
    CHUNK(W3D_CHUNK_PRELIT_VERTEX)
    CHUNK(W3D_CHUNK_PS2_SHADERS)
    CHUNK(W3D_CHUNK_SCG)
    CHUNK(W3D_CHUNK_SHADER_IDS)
    CHUNK(W3D_CHUNK_SHADERS)
    CHUNK(W3D_CHUNK_SKIN_NODE)
    CHUNK(W3D_CHUNK_SPOT_LIGHT_INFO)
    CHUNK(W3D_CHUNK_SPOT_LIGHT_INFO_5_0)
    CHUNK(W3D_CHUNK_STAGE_TEXCOORDS)
    CHUNK(W3D_CHUNK_SURRENDER_NORMALS)
    CHUNK(W3D_CHUNK_TEXCOORDS)
    CHUNK(W3D_CHUNK_TEXTURE)
    CHUNK(W3D_CHUNK_TEXTURE_IDS)
    CHUNK(W3D_CHUNK_TEXTURE_INFO)
    CHUNK(W3D_CHUNK_TEXTURE_NAME)
    CHUNK(W3D_CHUNK_TEXTURE_REPLACER_INFO)
    CHUNK(W3D_CHUNK_TEXTURE_STAGE)
    CHUNK(W3D_CHUNK_TEXTURES)
    CHUNK(W3D_CHUNK_TRANSFORM_NODE)
    CHUNK(W3D_CHUNK_TRIANGLES)
    CHUNK(W3D_CHUNK_VERTEX_COLORS)
    CHUNK(W3D_CHUNK_VERTEX_INFLUENCES)
    CHUNK(W3D_CHUNK_VERTEX_MAPPER_ARGS0)
    CHUNK(W3D_CHUNK_VERTEX_MAPPER_ARGS1)
    CHUNK(W3D_CHUNK_VERTEX_MATERIAL)
    CHUNK(W3D_CHUNK_VERTEX_MATERIAL_IDS)
    CHUNK(W3D_CHUNK_VERTEX_MATERIAL_INFO)
    CHUNK(W3D_CHUNK_VERTEX_MATERIAL_NAME)
    CHUNK(W3D_CHUNK_VERTEX_MATERIALS)
    CHUNK(W3D_CHUNK_VERTEX_NORMALS)
    CHUNK(W3D_CHUNK_VERTEX_SHADE_INDICES)
    CHUNK(W3D_CHUNK_VERTICES)
    CHUNK(W3D_CHUNK_EMITTER_LINE_PROPERTIES)
    CHUNK(W3D_CHUNK_SECONDARY_VERTICES)
    CHUNK(W3D_CHUNK_SECONDARY_VERTEX_NORMALS)
    CHUNK(W3D_CHUNK_TANGENTS)
    CHUNK(W3D_CHUNK_BINORMALS)
    CHUNK(W3D_CHUNK_COMPRESSED_ANIMATION)
    CHUNK(W3D_CHUNK_COMPRESSED_ANIMATION_HEADER)
    CHUNK(W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL)
    CHUNK(W3D_CHUNK_COMPRESSED_BIT_CHANNEL)
    CHUNK(W3D_CHUNK_MORPH_ANIMATION)
    CHUNK(W3D_CHUNK_MORPHANIM_HEADER)
    CHUNK(W3D_CHUNK_MORPHANIM_CHANNEL)
    CHUNK(W3D_CHUNK_MORPHANIM_POSENAME)
    CHUNK(W3D_CHUNK_MORPHANIM_KEYDATA)
    CHUNK(W3D_CHUNK_MORPHANIM_PIVOTCHANNELDATA)
    CHUNK(W3D_CHUNK_SOUNDROBJ)
    CHUNK(W3D_CHUNK_SOUNDROBJ_HEADER)
    CHUNK(W3D_CHUNK_SOUNDROBJ_DEFINITION)
    CHUNK(W3D_CHUNK_RING)
    CHUNK(W3D_CHUNK_SPHERE)
    CHUNK(W3D_CHUNK_SHDMESH)
    CHUNK(W3D_CHUNK_SHDMESH_NAME)
    CHUNK(W3D_CHUNK_SHDSUBMESH)
    CHUNK(W3D_CHUNK_SHDSUBMESH_SHADER)
    CHUNK(W3D_CHUNK_SHDSUBMESH_SHADER_TYPE)
    CHUNK(W3D_CHUNK_SHDSUBMESH_VERTICES)
    CHUNK(W3D_CHUNK_SHDSUBMESH_VERTEX_NORMALS)
    CHUNK(W3D_CHUNK_SHDSUBMESH_TRIANGLES)
    CHUNK(W3D_CHUNK_SHDSUBMESH_VERTEX_SHADE_INDICES)
    CHUNK(W3D_CHUNK_SHDSUBMESH_UV0)
    CHUNK(W3D_CHUNK_SHDSUBMESH_UV1)
    CHUNK(W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_S)
    CHUNK(W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_T)
    CHUNK(W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_SXT)
    CHUNK(W3D_CHUNK_EMITTER_EXTRA_INFO)
    CHUNK(W3D_CHUNK_SHDMESH_USER_TEXT)
    CHUNK(W3D_CHUNK_FXSHADER_IDS)
    CHUNK(W3D_CHUNK_FX_SHADERS)
    CHUNK(W3D_CHUNK_FX_SHADER)
    CHUNK(W3D_CHUNK_FX_SHADER_INFO)
    CHUNK(W3D_CHUNK_FX_SHADER_CONSTANT)
    CHUNK(W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL)
}
#pragma clang diagnostic pop