#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#include "chunksmanager.h"
#include <string>
#include <typeinfo>

// Helper Functions

char *ReadChunkData(ChunkLoadClass &cload)
{
    if (!cload.Cur_Chunk_Length()) {
        char *c = new char[1];
        c[0] = 0;
        return c;
    }
    char *c = new char[cload.Cur_Chunk_Length()];
    cload.Read(c, cload.Cur_Chunk_Length());
    return c;
}

void WriteChunkData(ChunkSaveClass &csave, unsigned id, const void *data, unsigned dataSize)
{
    csave.Begin_Chunk(id);
    csave.Write(data, dataSize);
    csave.End_Chunk(); // TODO ASSERT SIZE
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

#define DUMP_SERIALIZE_CHUNK_STRING(name) \
    void Dump_##name(ChunkLoadClass &cload, ChunkTreePtr &data) \
    { \
        char *chunkdata = ReadChunkData(cload); \
        AddString(data, #name, chunkdata, "string"); \
        delete[] chunkdata; \
    } \
\
    void Serialize_##name(ChunkSaveClass &csave, ChunkTreePtr &data) \
    { \
        char *chunkdata = (char *)data->data->info->value; \
        WriteChunkData(csave, name, chunkdata,  sizeof(chunkdata)); \
    }

#define DUMP_SERIALIZE_CHUNK(name, type) \
    void Dump_##name(ChunkLoadClass &cload, ChunkTreePtr &data) \
    { \
        char *chunkdata = ReadChunkData(cload); \
        type *chunkData = (type *)chunkdata; \
        AddData(data, #name, #type, "", (void *)chunkData); \
        delete[] chunkdata; \
    } \
\
    void Serialize_##name(ChunkSaveClass &csave, ChunkTreePtr &data) \
    { \
        type *chunkData = (type *)data->data->info->value; \
        WriteChunkData(csave, name, chunkData, sizeof(type)); \
    }

#define DUMP_SERIALIZE_CHUNK_ARRAY(name, type) \
    void Dump_##name(ChunkLoadClass &cload, ChunkTreePtr &data) \
    { \
        char *chunkdata = ReadChunkData(cload); \
        type *arrayData = (type *)chunkdata; \
        AddData(data, #name, #type "[]", "", (void *)arrayData); \
        delete[] chunkdata; \
    } \
\
    void Serialize_##name(ChunkSaveClass &csave, ChunkTreePtr &data) \
    { \
        type *arrayData = (type *)data->data->info->value; \
        unsigned numElements = data->data->chunkSize / sizeof(type); \
        WriteChunkData(csave, name, arrayData, numElements * sizeof(type)); \
    }

#define DUMP_SERIALIZE_PARSER(name) \
    void Dump_##name(ChunkLoadClass &cload, ChunkTreePtr &data) \
    { \
        AddString(data, #name, "", "string"); \
        ChunkManager::DumpSubChunks(cload, data); \
    } \
\
    void Serialize_##name(ChunkSaveClass &csave, ChunkTreePtr &data) \
    { \
        ChunkManager::SerializeSubChunks(csave, data); \
    }

// Example Replacement:
void Dump_PARSER_DUMMY(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    // spilt name from Dump___func__, no need in macros
    const char *name = strstr(__func__, "Dump_") + 5;
    AddString(data, name, "", "string");
    data->subchunks = std::vector<ChunkTreePtr>(1);
    ChunkManager::DumpSubChunks(cload, data);
}

void Dump_W3D_CHUNK_ARRAY_DUMMY(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMaterialStruct *arrayData = (W3dMaterialStruct *)chunkdata;
    AddData(data, "O_W3D_CHUNK_MATERIALS", "W3dMaterialStruct[]", "", (void *)arrayData);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_ARRAY_DUMMY(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dMaterialStruct *arrayData = (W3dMaterialStruct *)data->data->info->value;
    unsigned numElements = data->data->chunkSize / sizeof(W3dMaterialStruct);
    WriteChunkData(csave, O_W3D_CHUNK_MATERIALS, arrayData, numElements * sizeof(W3dMaterialStruct));
}

void Dump_W3D_CHUNK_PARSER_DUMMY(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    // Because data is parent chunk, we need to add the name of the chunk, no any data or value
    AddString(data, "W3D_CHUNK_ARRAY_DUMMY", "", "string");
    ChunkManager::DumpSubChunks(cload, data);
}

// Dump and Serialize Functions
DUMP_SERIALIZE_CHUNK_ARRAY(O_W3D_CHUNK_MATERIALS, W3dMaterialStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(O_W3D_CHUNK_MATERIALS2, W3dMaterial2Struct)

void Dump_O_W3D_CHUNK_POV_QUADRANGLES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_POV_QUADRANGLES is unsupported");
    char *chunkdata = ReadChunkData(cload);
    AddData(data, "O_W3D_CHUNK_POV_QUADRANGLES", "char", "string", (void *)chunkdata);
    delete[] chunkdata;
}

void Serialize_O_W3D_CHUNK_POV_QUADRANGLES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_POV_QUADRANGLES is unsupported");
    char *chunkdata = (char *)data->data->info->value;
    WriteChunkData(csave, O_W3D_CHUNK_POV_QUADRANGLES, chunkdata, data->data->chunkSize);
}

void Dump_O_W3D_CHUNK_POV_TRIANGLES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_POV_TRIANGLES is unsupported");
    char *chunkdata = ReadChunkData(cload);
    AddData(data, "O_W3D_CHUNK_POV_TRIANGLES", "char", "string", (void *)chunkdata);
    delete[] chunkdata;
}

void Serialize_O_W3D_CHUNK_POV_TRIANGLES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_POV_TRIANGLES is unsupported");
    char *chunkdata = (char *)data->data->info->value;
    WriteChunkData(csave, O_W3D_CHUNK_POV_TRIANGLES, chunkdata, data->data->chunkSize);
}

void Dump_O_W3D_CHUNK_QUADRANGLES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_QUADRANGLES is outdated");
    char *chunkdata = ReadChunkData(cload);
    AddData(data, "O_W3D_CHUNK_QUADRANGLES", "char", "string", (void *)chunkdata);
    delete[] chunkdata;
}

void Serialize_O_W3D_CHUNK_QUADRANGLES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_QUADRANGLES is outdated");
    char *chunkdata = (char *)data->data->info->value;
    WriteChunkData(csave, O_W3D_CHUNK_QUADRANGLES, chunkdata, data->data->chunkSize);
}

DUMP_SERIALIZE_CHUNK_ARRAY(O_W3D_CHUNK_SURRENDER_TRIANGLES, W3dSurrenderTriangleStruct)

void Dump_O_W3D_CHUNK_TRIANGLES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_TRIANGLES is obsoleted");
    char *chunkdata = ReadChunkData(cload);
    AddData(data, "O_W3D_CHUNK_TRIANGLES", "char", "string", (void *)chunkdata);
    delete[] chunkdata;
}

void Serialize_O_W3D_CHUNK_TRIANGLES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    captainslog_warn("O_W3D_CHUNK_TRIANGLES is obsoleted");
    char *chunkdata = (char *)data->data->info->value;
    WriteChunkData(csave, O_W3D_CHUNK_TRIANGLES, chunkdata, data->data->chunkSize);
}

DUMP_SERIALIZE_CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_COLOR_KEYFRAME, W3dEmitterColorKeyframeStruct)

DUMP_SERIALIZE_CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME, W3dEmitterOpacityKeyframeStruct)

DUMP_SERIALIZE_CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_SIZE_KEYFRAME, W3dEmitterSizeKeyframeStruct)

DUMP_SERIALIZE_CHUNK(OBSOLETE_W3D_CHUNK_SHADOW_NODE, W3dHModelNodeStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_AABTREE)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_AABTREE_HEADER, W3dMeshAABTreeHeader)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_AABTREE_NODES, W3dMeshAABTreeNode)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_AABTREE_POLYINDICES, uint32_t)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_AGGREGATE)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_AGGREGATE_CLASS_INFO, W3dAggregateMiscInfo)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_AGGREGATE_HEADER, W3dAggregateHeaderStruct)

void Dump_W3D_CHUNK_AGGREGATE_INFO(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dAggregateInfoStruct *info = (W3dAggregateInfoStruct *)chunkdata;
    W3dAggregateSubobjectStruct *subobj = (W3dAggregateSubobjectStruct *)(chunkdata + sizeof(W3dAggregateInfoStruct));

    // TODO: Add Subobjects
    captainslog_warn("W3D_CHUNK_AGGREGATE_INFO Subobjects are not correctly implemented");
    AddData(data, "W3D_CHUNK_AGGREGATE_INFO", "W3dAggregateInfoStruct[][]", "Obsolete", (void *)info);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_AGGREGATE_INFO(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dAggregateInfoStruct *info = (W3dAggregateInfoStruct *)data->data->info->value;
    WriteChunkData(csave, W3D_CHUNK_AGGREGATE_INFO, info, sizeof(W3dAggregateInfoStruct));
}

DUMP_SERIALIZE_PARSER(W3D_CHUNK_ANIMATION)

void Dump_W3D_CHUNK_ANIMATION_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dAnimChannelStruct *channel = (W3dAnimChannelStruct *)chunkdata;

    if (channel->Flags <= ANIM_CHANNEL_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    AddData(data, "W3D_CHUNK_ANIMATION_CHANNEL", "W3dAnimChannelStruct", "", (void *)channel);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_ANIMATION_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dAnimChannelStruct *channel = (W3dAnimChannelStruct *)data->data->info->value;
    if (channel->Flags <= ANIM_CHANNEL_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    WriteChunkData(csave, W3D_CHUNK_ANIMATION_CHANNEL, channel, sizeof(W3dAnimChannelStruct));
}

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_ANIMATION_HEADER, W3dAnimHeaderStruct)

void Dump_W3D_CHUNK_BIT_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dBitChannelStruct *channel = (W3dBitChannelStruct *)chunkdata;
    if (channel->Flags <= BIT_CHANNEL_TIMECODED_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_BIT_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    AddData(data, "W3D_CHUNK_BIT_CHANNEL", "W3dBitChannelStruct", "", (void *)channel);
    delete[] chunkdata;
}
void Serialize_W3D_CHUNK_BIT_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dBitChannelStruct *channel = (W3dBitChannelStruct *)data->data->info->value;
    if (channel->Flags <= BIT_CHANNEL_TIMECODED_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_BIT_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    WriteChunkData(csave, W3D_CHUNK_BIT_CHANNEL, channel, sizeof(W3dBitChannelStruct));
}

void Dump_W3D_CHUNK_BOX(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dBoxStruct *box = (W3dBoxStruct *)chunkdata;

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

    AddData(data, "W3D_CHUNK_BOX", "W3dBoxStruct", "", (void *)box);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_BOX(ChunkSaveClass &csave, ChunkTreePtr &data)
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

    WriteChunkData(csave, W3D_CHUNK_BOX, box, sizeof(W3dBoxStruct));
}

DUMP_SERIALIZE_PARSER(W3D_CHUNK_COLLECTION)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_COLLECTION_HEADER, W3dCollectionHeaderStruct)

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_COLLECTION_OBJ_NAME)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_COLLISION_NODE, W3dHModelNodeStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_DAMAGE)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_DAMAGE_COLORS, W3dDamageColorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_DAMAGE_HEADER, W3dDamageStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_DAMAGE_VERTICES, W3dDamageVertexStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_DAZZLE)

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_DAZZLE_NAME)

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_DAZZLE_TYPENAME)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_DCG, W3dRGBAStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_DIG, W3dRGBStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_EMITTER)

void Dump_W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterBlurTimeHeaderStruct *header = (W3dEmitterBlurTimeHeaderStruct *)chunkdata;

    W3dEmitterBlurTimeKeyframeStruct *blurtime =
        (W3dEmitterBlurTimeKeyframeStruct *)(chunkdata + sizeof(W3dEmitterBlurTimeHeaderStruct));

    // TODO: Add subobjects
    captainslog_warn("W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES Subobjects are not correctly implemented");
    AddData(data, "W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES", "W3dEmitterBlurTimeHeaderStruct[][]", "", (void *)chunkdata);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dEmitterBlurTimeHeaderStruct *header = (W3dEmitterBlurTimeHeaderStruct *)data->data->info->value;
    WriteChunkData(csave, W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES, header, sizeof(W3dEmitterBlurTimeHeaderStruct));
}

void Dump_W3D_CHUNK_EMITTER_FRAME_KEYFRAMES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterFrameHeaderStruct *header = (W3dEmitterFrameHeaderStruct *)chunkdata;
    W3dEmitterFrameKeyframeStruct *frame =
        (W3dEmitterFrameKeyframeStruct *)(chunkdata + sizeof(W3dEmitterFrameHeaderStruct));

    // TODO: Add subobjects
    captainslog_warn("W3D_CHUNK_EMITTER_FRAME_KEYFRAMES Subobjects are not correctly implemented");
    AddData(data, "W3D_CHUNK_EMITTER_FRAME_KEYFRAMES", "W3dEmitterFrameHeaderStruct[][]", "", (void *)chunkdata);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_EMITTER_FRAME_KEYFRAMES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dEmitterFrameHeaderStruct *header = (W3dEmitterFrameHeaderStruct *)data->data->info->value;
    WriteChunkData(csave, W3D_CHUNK_EMITTER_FRAME_KEYFRAMES, header, sizeof(W3dEmitterFrameHeaderStruct));
}

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_EMITTER_HEADER, W3dEmitterHeaderStruct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_EMITTER_INFO, W3dEmitterInfoStruct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_EMITTER_INFOV2, W3dEmitterInfoStructV2)

void Dump_W3D_CHUNK_EMITTER_PROPS(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterPropertyStruct *props = (W3dEmitterPropertyStruct *)chunkdata;

    W3dEmitterColorKeyframeStruct *color = (W3dEmitterColorKeyframeStruct *)(chunkdata + sizeof(W3dEmitterPropertyStruct));

    W3dEmitterOpacityKeyframeStruct *opacity = (W3dEmitterOpacityKeyframeStruct *)(chunkdata
        + sizeof(W3dEmitterPropertyStruct) + (props->ColorKeyframes * sizeof(W3dEmitterColorKeyframeStruct)));

    W3dEmitterSizeKeyframeStruct *size = (W3dEmitterSizeKeyframeStruct *)(chunkdata + sizeof(W3dEmitterPropertyStruct)
        + (props->ColorKeyframes * sizeof(W3dEmitterColorKeyframeStruct))
        + (props->OpacityKeyframes * sizeof(W3dEmitterOpacityKeyframeStruct)));

    // TODO: Add subobjects
    captainslog_warn("W3D_CHUNK_EMITTER_PROPS Subobjects are not correctly implemented");
    AddData(data, "W3D_CHUNK_EMITTER_PROPS", "W3dEmitterPropertyStruct[][][][]", "", (void *)chunkdata);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_EMITTER_PROPS(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dEmitterPropertyStruct *props = (W3dEmitterPropertyStruct *)data->data->info->value;
    WriteChunkData(csave, W3D_CHUNK_EMITTER_PROPS, props, sizeof(W3dEmitterPropertyStruct));
}

void Dump_W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterRotationHeaderStruct *header = (W3dEmitterRotationHeaderStruct *)chunkdata;

    W3dEmitterRotationKeyframeStruct *frame =
        (W3dEmitterRotationKeyframeStruct *)(chunkdata + sizeof(W3dEmitterRotationHeaderStruct));

    // TODO: Add subobjects
    captainslog_warn("W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES Subobjects are not correctly implemented");
    AddData(data, "W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES", "W3dEmitterRotationHeaderStruct[][]", "", (void *)chunkdata);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dEmitterRotationHeaderStruct *header = (W3dEmitterRotationHeaderStruct *)data->data->info->value;
    WriteChunkData(csave, W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES, header, sizeof(W3dEmitterRotationHeaderStruct));
}

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_EMITTER_USER_DATA)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_FAR_ATTENUATION, W3dLightAttenuationStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_HIERARCHY)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_HIERARCHY_HEADER, W3dHierarchyStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_HLOD)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_HLOD_AGGREGATE_ARRAY)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_HLOD_HEADER, W3dHLodHeaderStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_HLOD_LOD_ARRAY)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER, W3dHLodArrayHeaderStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_HLOD_PROXY_ARRAY)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_HLOD_SUB_OBJECT, W3dHLodSubObjectStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_HMODEL)

DUMP_SERIALIZE_CHUNK(OBSOLETE_W3D_CHUNK_HMODEL_AUX_DATA, W3dHModelAuxDataStruct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_HMODEL_HEADER, W3dHModelHeaderStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_LIGHT)

void Dump_W3D_CHUNK_LIGHT_INFO(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dLightStruct *light = (W3dLightStruct *)chunkdata;

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
    AddData(data, "W3D_CHUNK_LIGHT_INFO", "W3dLightStruct", "", (void *)chunkdata);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_LIGHT_INFO(ChunkSaveClass &csave, ChunkTreePtr &data)
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

    WriteChunkData(csave, W3D_CHUNK_LIGHT_INFO, light, sizeof(W3dLightStruct));
}

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_LIGHT_TRANSFORM, W3dLightTransformStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_LIGHTSCAPE)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_LIGHTSCAPE_LIGHT)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_LOD, W3dLODStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_LODMODEL)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_LODMODEL_HEADER, W3dLODModelHeaderStruct)

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_MAP3_FILENAME)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_MAP3_INFO, W3dMap3Struct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_MATERIAL_INFO, W3dMaterialInfoStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MATERIAL_PASS)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MATERIAL3)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MATERIAL3_DC_MAP)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MATERIAL3_DI_MAP)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_MATERIAL3_INFO, W3dMaterial3Struct)

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_MATERIAL3_NAME)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MATERIAL3_SC_MAP)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MATERIAL3_SI_MAP)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MATERIALS3)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MESH)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_MESH_HEADER, W3dMeshHeaderStruct)

void Dump_W3D_CHUNK_MESH_HEADER3(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMeshHeader3Struct *header = (W3dMeshHeader3Struct *)chunkdata;
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
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_MESH_HEADER3(ChunkSaveClass &csave, ChunkTreePtr &data)
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

    WriteChunkData(csave, W3D_CHUNK_MESH_HEADER3, header, sizeof(W3dMeshHeader3Struct));
}

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_MESH_USER_TEXT)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_NEAR_ATTENUATION, W3dLightAttenuationStruct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_NODE, W3dHModelNodeStruct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_NULL_OBJECT, W3dNullObjectStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_PER_FACE_TEXCOORD_IDS, Vector3i)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_PER_TRI_MATERIALS, uint16_t)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_PIVOT_FIXUPS, W3dPivotFixupStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_PIVOTS, W3dPivotStruct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_PLACEHOLDER, W3dPlaceholderStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_POINTS, W3dVectorStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_PRELIT_UNLIT)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_PRELIT_VERTEX)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_PS2_SHADERS, W3dPS2ShaderStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SCG, W3dRGBStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SHADER_IDS, uint32_t)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SHADERS, W3dShaderStruct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_SKIN_NODE, W3dHModelNodeStruct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_SPOT_LIGHT_INFO, W3dSpotLightStruct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_SPOT_LIGHT_INFO_5_0, W3dSpotLightStruct_v5_0)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_STAGE_TEXCOORDS, W3dTexCoordStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SURRENDER_NORMALS, W3dVectorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_TEXCOORDS, W3dTexCoordStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_TEXTURE)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_TEXTURE_IDS, uint32_t)

void Dump_W3D_CHUNK_TEXTURE_INFO(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dTextureInfoStruct *info = (W3dTextureInfoStruct *)chunkdata;

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
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_TEXTURE_INFO(ChunkSaveClass &csave, ChunkTreePtr &data)
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

    WriteChunkData(csave, W3D_CHUNK_TEXTURE_INFO, info, sizeof(W3dTextureInfoStruct));
}

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_TEXTURE_NAME)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_TEXTURE_REPLACER_INFO, W3dTextureReplacerStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_TEXTURE_STAGE)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_TEXTURES)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_TRANSFORM_NODE, W3dPlaceholderStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_TRIANGLES, W3dTriStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_VERTEX_COLORS, W3dRGBStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_VERTEX_INFLUENCES, W3dVertInfStruct)

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_VERTEX_MAPPER_ARGS0)

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_VERTEX_MAPPER_ARGS1)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_VERTEX_MATERIAL)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_VERTEX_MATERIAL_IDS, uint32_t)

void Dump_W3D_CHUNK_VERTEX_MATERIAL_INFO(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dVertexMaterialStruct *material = (W3dVertexMaterialStruct *)chunkdata;

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
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_VERTEX_MATERIAL_INFO(ChunkSaveClass &csave, ChunkTreePtr &data)
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

    WriteChunkData(csave, W3D_CHUNK_VERTEX_MATERIAL_INFO, material, sizeof(W3dVertexMaterialStruct));
}

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_VERTEX_MATERIAL_NAME)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_VERTEX_MATERIALS)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_VERTEX_NORMALS, W3dVectorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_VERTEX_SHADE_INDICES, uint32_t)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_VERTICES, W3dVectorStruct)

void Dump_W3D_CHUNK_EMITTER_LINE_PROPERTIES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterLinePropertiesStruct *props = (W3dEmitterLinePropertiesStruct *)chunkdata;

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
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_EMITTER_LINE_PROPERTIES(ChunkSaveClass &csave, ChunkTreePtr &data)
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

    WriteChunkData(csave, W3D_CHUNK_EMITTER_LINE_PROPERTIES, props, sizeof(W3dEmitterLinePropertiesStruct));
}

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SECONDARY_VERTICES, W3dVectorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SECONDARY_VERTEX_NORMALS, W3dVectorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_TANGENTS, W3dVectorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_BINORMALS, W3dVectorStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_COMPRESSED_ANIMATION)

int flavor = ANIM_FLAVOR_TIMECODED;

void Dump_W3D_CHUNK_COMPRESSED_ANIMATION_HEADER(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dCompressedAnimHeaderStruct *header = (W3dCompressedAnimHeaderStruct *)chunkdata;

    if (header->Flavor >= ANIM_FLAVOR_VALID) {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_HEADER Unknown Flavor Type %x", header->Flavor);
        captainslog_warn((const char *)str);
    }

    flavor = header->Flavor;
    AddData(data, "W3D_CHUNK_COMPRESSED_ANIMATION_HEADER", "W3dCompressedAnimHeaderStruct", "", (void *)header);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_COMPRESSED_ANIMATION_HEADER(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dCompressedAnimHeaderStruct *header = (W3dCompressedAnimHeaderStruct *)data->data->info->value;

    if (header->Flavor >= ANIM_FLAVOR_VALID) {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_HEADER Unknown Flavor Type %x", header->Flavor);
        captainslog_warn((const char *)str);
    }

    flavor = header->Flavor;
    WriteChunkData(csave, W3D_CHUNK_COMPRESSED_ANIMATION_HEADER, header, sizeof(W3dCompressedAnimHeaderStruct));
}

void Dump_W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);

    if (flavor == ANIM_FLAVOR_TIMECODED) {
        W3dTimeCodedAnimChannelStruct *channel = (W3dTimeCodedAnimChannelStruct *)chunkdata;

        if (channel->Flags <= ANIM_CHANNEL_VIS) {
        } else {
            StringClass str;
            str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
            captainslog_warn((const char *)str);
        }
        AddData(data, "W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL", "W3dTimeCodedAnimChannelStruct", "", (void *)channel);
        delete[] chunkdata;
    } else {
        W3dAdaptiveDeltaAnimChannelStruct *channel = (W3dAdaptiveDeltaAnimChannelStruct *)chunkdata;

        if (channel->Flags <= ANIM_CHANNEL_VIS) {
        } else {
            StringClass str;
            str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
            captainslog_warn((const char *)str);
        }

        AddData(data, "W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL", "W3dAdaptiveDeltaAnimChannelStruct", "", (void *)channel);
        delete[] chunkdata;
    }
}

void Serialize_W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    if (flavor == ANIM_FLAVOR_TIMECODED) {
        W3dTimeCodedAnimChannelStruct *channel = (W3dTimeCodedAnimChannelStruct *)data->data->info->value;
        if (channel->Flags <= ANIM_CHANNEL_VIS) {
        } else {
            StringClass str;
            str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
            captainslog_warn((const char *)str);
        }
        WriteChunkData(csave, W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL, channel, sizeof(W3dTimeCodedAnimChannelStruct));
    } else {
        W3dAdaptiveDeltaAnimChannelStruct *channel = (W3dAdaptiveDeltaAnimChannelStruct *)data->data->info->value;
        if (channel->Flags <= ANIM_CHANNEL_VIS) {
        } else {
            StringClass str;
            str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
            captainslog_warn((const char *)str);
        }
        WriteChunkData(csave, W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL, channel, sizeof(W3dAdaptiveDeltaAnimChannelStruct));
    }
}

void Dump_W3D_CHUNK_COMPRESSED_BIT_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dTimeCodedBitChannelStruct *channel = (W3dTimeCodedBitChannelStruct *)chunkdata;

    if (channel->Flags > BIT_CHANNEL_TIMECODED_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_BIT_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    AddData(data, "W3D_CHUNK_COMPRESSED_BIT_CHANNEL", "W3dTimeCodedBitChannelStruct", "", (void *)channel);

    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_COMPRESSED_BIT_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dTimeCodedBitChannelStruct *channel = (W3dTimeCodedBitChannelStruct *)data->data->info->value;
    if (channel->Flags > BIT_CHANNEL_TIMECODED_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_BIT_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    WriteChunkData(csave, W3D_CHUNK_COMPRESSED_BIT_CHANNEL, channel, sizeof(W3dTimeCodedBitChannelStruct));
}

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MORPH_ANIMATION)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_MORPHANIM_HEADER, W3dMorphAnimHeaderStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MORPHANIM_CHANNEL)

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_MORPHANIM_POSENAME)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_MORPHANIM_KEYDATA, W3dMorphAnimKeyStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_MORPHANIM_PIVOTCHANNELDATA, uint32_t)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_SOUNDROBJ)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_SOUNDROBJ_HEADER, W3dSoundRObjHeaderStruct)

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

void Dump_W3D_CHUNK_SOUNDROBJ_DEFINITION(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    // TODO: Implement Dump_W3D_CHUNK_SOUNDROBJ_DEFINITION
    captainslog_warn("Dump_W3D_CHUNK_SOUNDROBJ_DEFINITION not implemented");
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

void Serialize_W3D_CHUNK_SOUNDROBJ_DEFINITION(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_SOUNDROBJ_DEFINITION
    captainslog_warn("Serialize_W3D_CHUNK_SOUNDROBJ_DEFINITION not implemented");
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

void Dump_W3D_CHUNK_RING(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    // TODO: Implement Dump_W3D_CHUNK_RING
    captainslog_warn("Dump_W3D_CHUNK_RING not implemented");
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

void Serialize_W3D_CHUNK_RING(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_RING
    captainslog_warn("Serialize_W3D_CHUNK_RING not implemented");
}

void Dump_W3D_CHUNK_SPHERE(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    // TODO: Implement Dump_W3D_CHUNK_SPHERE
    captainslog_warn("Dump_W3D_CHUNK_SPHERE not implemented");
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

void Serialize_W3D_CHUNK_SPHERE(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_SPHERE
    captainslog_warn("Serialize_W3D_CHUNK_SPHERE not implemented");
}

DUMP_SERIALIZE_PARSER(W3D_CHUNK_SHDMESH)

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_SHDMESH_NAME)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_SHDSUBMESH)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_SHDSUBMESH_SHADER)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_SHDSUBMESH_SHADER_TYPE, uint32_t)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_VERTICES, W3dVectorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_VERTEX_NORMALS, W3dVectorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_TRIANGLES, Vector3i16)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_VERTEX_SHADE_INDICES, uint32_t)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_UV0, W3dTexCoordStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_UV1, W3dTexCoordStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_S, W3dVectorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_T, W3dVectorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_SXT, W3dVectorStruct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_EMITTER_EXTRA_INFO, W3dEmitterExtraInfoStruct)

DUMP_SERIALIZE_CHUNK_STRING(W3D_CHUNK_SHDMESH_USER_TEXT)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_FXSHADER_IDS, uint32_t)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_FX_SHADERS)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_FX_SHADER)

void Dump_W3D_CHUNK_FX_SHADER_INFO(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    // TODO Fix W3dFXShaderStruct
    captainslog_warn("Dump_W3D_CHUNK_FX_SHADER_INFO not implemented");
    char *chunkdata = ReadChunkData(cload);
    uint8_t *version = (uint8_t *)chunkdata;
    AddInt8(data, "Version", *version);
    W3dFXShaderStruct *shader = (W3dFXShaderStruct *)(chunkdata + 1);
    AddString(data, "ShaderName", shader->shadername, "string");
    AddInt8(data, "Technique", shader->technique);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_FX_SHADER_INFO(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_FX_SHADER_INFO
    captainslog_warn("Serialize_W3D_CHUNK_FX_SHADER_INFO not implemented");
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

    W3dFXShaderConstantStruct &operator=(const char *chunkdata)
    {
        Type = *(uint32_t *)chunkdata;
        Length = *(uint32_t *)(chunkdata + 4);
        ConstantName = std::string(chunkdata + 8, Length);

        if (Type == CONSTANT_TYPE_TEXTURE) {
            Value.Texture = std::string(chunkdata + 8 + Length);
        } else if (Type >= CONSTANT_TYPE_FLOAT1 && Type <= CONSTANT_TYPE_FLOAT4) {
            int count = Type - 1;
            float *floats = (float *)(chunkdata + 8 + Length);
            Value.Floats.assign(floats, floats + count);
        } else if (Type == CONSTANT_TYPE_INT) {
            Value.Int = *(uint32_t *)(chunkdata + 8 + Length);
        } else if (Type == CONSTANT_TYPE_BOOL) {
            Value.Bool = *(uint8_t *)(chunkdata + 8 + Length) != 0;
        } else {
            // Handle unknown types
            Value.Texture = "Unknown";
        }

        return *this;
    }
};

void Dump_W3D_CHUNK_FX_SHADER_CONSTANT(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dFXShaderConstantStruct *constant = (W3dFXShaderConstantStruct *)chunkdata;
    if (constant->Value.Texture == "Unknown") {
        StringClass str;
        str.Format("W3D_CHUNK_FX_SHADER_CONSTANT Unknown Constant Type %x", constant->Type);
        captainslog_warn((const char *)str);
    }
    AddData(data, "W3D_CHUNK_FX_SHADER_CONSTANT", "W3dFXShaderConstantStruct", "", (void *)constant);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_FX_SHADER_CONSTANT(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    W3dFXShaderConstantStruct *constant = (W3dFXShaderConstantStruct *)data->data->info->value;
    if (constant->Value.Texture == "Unknown") {
        StringClass str;
        str.Format("W3D_CHUNK_FX_SHADER_CONSTANT Unknown Constant Type %x", constant->Type);
        captainslog_warn((const char *)str);
    }
    WriteChunkData(csave, W3D_CHUNK_FX_SHADER_CONSTANT, constant, sizeof(W3dFXShaderConstantStruct));
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

    W3dCompressedMotionChannelStructNew &operator=(const char *chunkdata)
    {
        Zero = *(uint8_t *)chunkdata;
        Flavor = *(uint8_t *)(chunkdata + 1);
        VectorLen = *(uint8_t *)(chunkdata + 2);
        Flags = *(uint8_t *)(chunkdata + 3);
        NumTimeCodes = *(uint16_t *)(chunkdata + 4);
        Pivot = *(uint16_t *)(chunkdata + 6);

        size_t offset = sizeof(W3dCompressedMotionChannelStructNew);

        if (Flavor == ANIM_FLAVOR_NEW_TIMECODED) {
            KeyFrames.assign(
                (uint16_t *)(chunkdata + offset), (uint16_t *)(chunkdata + offset + NumTimeCodes * sizeof(uint16_t)));
            offset += NumTimeCodes * sizeof(uint16_t);

            int datalen = VectorLen * NumTimeCodes;
            if (NumTimeCodes & 1) {
                offset += 2; // Padding for odd NumTimeCodes
            }
            Data.assign((uint32_t *)(chunkdata + offset), (uint32_t *)(chunkdata + offset + datalen * sizeof(uint32_t)));
        } else {
            Scale = *(float *)(chunkdata + offset);
            offset += sizeof(float);

            Initial.assign((float *)(chunkdata + offset), (float *)(chunkdata + offset + VectorLen * sizeof(float)));
            offset += VectorLen * sizeof(float);

            int count = (NumTimeCodes * VectorLen - sizeof(W3dCompressedMotionChannelStructNew) - sizeof(float)
                            - sizeof(float) * VectorLen)
                / sizeof(uint32_t);
            Data.assign((uint32_t *)(chunkdata + offset), (uint32_t *)(chunkdata + offset + count * sizeof(uint32_t)));
        }
        return *this;
    }
};

void Dump_W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dCompressedMotionChannelStructNew *channel = (W3dCompressedMotionChannelStructNew *)chunkdata;
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
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
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
    WriteChunkData(
        csave, W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL, channel, sizeof(W3dCompressedMotionChannelStructNew));
}

std::map<int, ChunkIOFuncs> chunkFuncMap;

#define CHUNK(id) \
    { \
        ChunkIOFuncs c; \
        c.name = #id; \
        c.ReadFunc = Dump_##id; \
        c.WriteFunc = Serialize_##id; \
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