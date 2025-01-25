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

void AddData(ChunkTreePtr &data, const char *name, const StringClass &type, const StringClass &formattedValue, void *value)
{
    if (data->data->info == nullptr) {
        data->data->info = std::move(ChunkManager::createChunkInfo(name, type, formattedValue, value));
    } else {
        captainslog_error("ChunkInfo %s already exists, cannot set %s", (const char *)data->data->info->name, name);
    }
}

void AddString(ChunkTreePtr &data, const char *name, const StringClass &value, const StringClass &type)
{
    AddData(data, name, type, value, nullptr);
}

void AddVersion(ChunkTreePtr &data, uint32_t value)
{
    char c[64];
    sprintf(c, "%u.%hu", value >> 16, (uint16_t)value);
    AddData(data, "Version", "uint32_t", c, (void *)&value);
}

void AddInt32(ChunkTreePtr &data, const char *name, uint32_t value)
{
    char c[256];
    sprintf(c, "%u", value);
    AddData(data, name, "uint32_t", c, (void *)&value);
}

void AddInt16(ChunkTreePtr &data, const char *name, uint16_t value)
{
    char c[256];
    sprintf(c, "%hu", value);
    AddData(data, name, "uint16_t", c, (void *)&value);
}

void AddInt8(ChunkTreePtr &data, const char *name, uint8_t value)
{
    char c[256];
    sprintf(c, "%hhu", value);
    AddData(data, name, "uint8_t", c, (void *)&value);
}

void AddInt8Array(ChunkTreePtr &data, const char *name, const uint8_t *values, int count)
{
    StringClass str;
    StringClass str2;
    for (int i = 0; i < count; i++) {
        str2.Format("%hhu ", values[i]);
        str += str2;
    }
    char c[256];
    sprintf(c, "int8[%d]", count);
    AddData(data, name, c, str, (void *)values);
}

void AddFloat(ChunkTreePtr &data, const char *name, float value)
{
    char c[256];
    sprintf(c, "%f", value);
    AddData(data, name, "float", c, (void *)&value);
}

void AddInt32Array(ChunkTreePtr &data, const char *name, const uint32_t *values, int count)
{
    StringClass str;
    StringClass str2;
    for (int i = 0; i < count; i++) {
        str2.Format("%u ", values[i]);
        str += str2;
    }
    char c[256];
    sprintf(c, "int32[%d]", count);
    AddData(data, name, c, str, (void *)values);
}

void AddFloatArray(ChunkTreePtr &data, const char *name, const float *values, int count)
{
    StringClass str;
    StringClass str2;
    for (int i = 0; i < count; i++) {
        str2.Format("%f ", values[i]);
        str += str2;
    }
    char c[256];
    sprintf(c, "float[%d]", count);
    AddData(data, name, c, str, (void *)values);
}

void AddVector(ChunkTreePtr &data, const char *name, const W3dVectorStruct *value)
{
    char c[256];
    sprintf(c, "%f %f %f", value->x, value->y, value->z);
    AddData(data, name, "W3dVectorStruct", c, (void *)value);
}

void AddQuaternion(ChunkTreePtr &data, const char *name, const W3dQuaternionStruct *value)
{
    char c[256];
    sprintf(c, "%f %f %f %f", value->q[0], value->q[1], value->q[2], value->q[3]);
    AddData(data, name, "W3dQuaternionStruct", c, (void *)value);
}

void AddRGB(ChunkTreePtr &data, const char *name, const W3dRGBStruct *value)
{
    StringClass str;
    str.Format("(%hhu %hhu %hhu) ", value->r, value->g, value->b);
    AddData(data, name, "W3dRGBStruct", str, (void *)value);
}

void AddRGBArray(ChunkTreePtr &data, const char *name, const W3dRGBStruct *values, int count)
{
    StringClass str;
    StringClass str2;
    for (int i = 0; i < count; i++) {
        str2.Format("(%hhu %hhu %hhu) ", values[i].r, values[i].g, values[i].b);
        str += str2;
    }
    char c[256];
    sprintf(c, "float[%d]", count);
    AddData(data, name, c, str, (void *)values);
}

void AddRGBA(ChunkTreePtr &data, const char *name, const W3dRGBAStruct *value)
{
    StringClass str;
    str.Format("(%hhu %hhu %hhu %hhu) ", value->R, value->G, value->B, value->A);
    AddData(data, name, "W3dRGBAStruct", str, (void *)value);
}

void AddTexCoord(ChunkTreePtr &data, const char *name, const W3dTexCoordStruct *value)
{
    char c[256];
    sprintf(c, "%f %f", value->U, value->V);
    AddData(data, name, "W3dTexCoordStruct", c, (void *)value);
}

void AddTexCoordArray(ChunkTreePtr &data, const char *name, const W3dTexCoordStruct *values, int count)
{
    StringClass str;
    for (int i = 0; i < count; i++) {
        char c[256];
        sprintf(c, "%s.TexCoord[%d]", name, i);
        AddTexCoord(data, c, &values[i]);
    }
}

const char *DepthCompareValues[] = { "Pass Never",
    "Pass Less",
    "Pass Equal",
    "Pass Less or Equal",
    "Pass Greater",
    "Pass Not Equal",
    "Pass Greater or Equal",
    "Pass Always" };

const char *DepthMaskValues[] = { "Write Disable", "Write Enable", "Write Disable", "Write Enable" };

const char *DestBlendValues[] = { "Zero",
    "One",
    "Src Color",
    "One Minus Src Color",
    "Src Alpha",
    "One Minus Src Alpha",
    "Src Color Prefog",
    "Disable",
    "Enable",
    "Scale Fragment",
    "Replace Fragment" };

const char *PriGradientValues[] = {
    "Disable", "Modulate", "Add", "Bump-Environment", "Bump-Environment Luminance", "Modulate 2x"
};

const char *SecGradientValues[] = { "Disable", "Enable" };

const char *SrcBlendValues[] = { "Zero", "One", "Src Alpha", "One Minus Src Alpha" };

const char *TexturingValues[] = { "Disable", "Enable" };

const char *DetailColorValues[] = { "Disable",
    "Detail",
    "Scale",
    "InvScale",
    "Add",
    "Sub",
    "SubR",
    "Blend",
    "DetailBlend",
    "Add Signed",
    "Add Signed 2x",
    "Scale 2x",
    "Mod Alpha Add Color" };

const char *DetailAlphaValues[] = { "Disable", "Detail", "Scale", "InvScale", "Disable", "Enable", "Smooth", "Flat" };

const char *AlphaTestValues[] = { "Alpha Test Disable", "Alpha Test Enable" };

void AddShader(ChunkTreePtr &data, const char *name, const W3dShaderStruct *value)
{
    // main shader
    AddData(data, name, "W3dShaderStruct", "Main Shader", (void *)value);
    char c[256];
    sprintf(c, "%s.DepthCompare", name);

    if (value->DepthCompare < W3DSHADER_DEPTHCOMPARE_PASS_MAX) {
        AddString(data, c, DepthCompareValues[value->DepthCompare], "uint8_t");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Depth Compare type %x", c, value->DepthCompare);
        captainslog_warn("Unknown chunk: %s", (const char *)str);
        AddData(data, c, "Unknown", "uint8_t", (void *)&value->DepthCompare);
    }

    sprintf(c, "%s.DepthMask", name);

    if (value->DepthMask < W3DSHADER_DEPTHMASK_WRITE_MAX) {
        AddString(data, c, DepthMaskValues[value->DepthMask], "uint8_t");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Depth Mask type %x", c, value->DepthMask);
        captainslog_warn("Unknown chunk: %s", (const char *)str);
        AddData(data, c, "Unknown", "uint8_t", (void *)&value->DepthMask);
    }

    sprintf(c, "%s.DestBlend", name);

    if (value->DestBlend < W3DSHADER_DESTBLENDFUNC_MAX) {
        AddString(data, c, DestBlendValues[value->DestBlend], "uint8_t");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Dest Blend type %x", c, value->DestBlend);
        captainslog_warn("Unknown chunk: %s", (const char *)str);
        AddData(data, c, "Unknown", "uint8_t", (void *)&value->DestBlend);
    }

    sprintf(c, "%s.PriGradient", name);

    if (value->PriGradient < W3DSHADER_PRIGRADIENT_MAX) {
        AddString(data, c, PriGradientValues[value->PriGradient], "uint8_t");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Primary Gradient type %x", c, value->PriGradient);
        captainslog_warn("Unknown chunk: %s", (const char *)str);
        AddData(data, c, "Unknown", "uint8_t", (void *)&value->PriGradient);
    }

    sprintf(c, "%s.SecGradient", name);

    if (value->SecGradient < W3DSHADER_SECGRADIENT_MAX) {
        AddString(data, c, SecGradientValues[value->SecGradient], "uint8_t");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Secondary Gradient type %x", c, value->SecGradient);
        captainslog_warn("Unknown chunk: %s", (const char *)str);
        AddData(data, c, "Unknown", "uint8_t", (void *)&value->SecGradient);
    }

    sprintf(c, "%s.SrcBlend", name);

    if (value->SrcBlend < W3DSHADER_SRCBLENDFUNC_MAX) {
        AddString(data, c, SrcBlendValues[value->SrcBlend], "uint8_t");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Src Blend type %x", c, value->SrcBlend);
        captainslog_warn("Unknown chunk: %s", (const char *)str);
        AddData(data, c, "Unknown", "uint8_t", (void *)&value->SrcBlend);
    }

    sprintf(c, "%s.Texturing", name);

    if (value->Texturing < W3DSHADER_TEXTURING_MAX) {
        AddString(data, c, TexturingValues[value->Texturing], "uint8_t");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Texturing type %x", c, value->Texturing);
        captainslog_warn("Unknown chunk: %s", (const char *)str);
        AddData(data, c, "Unknown", "uint8_t", (void *)&value->Texturing);
    }

    sprintf(c, "%s.DetailColor", name);

    if (value->DetailColorFunc < W3DSHADER_DETAILCOLORFUNC_MAX) {
        AddString(data, c, DetailColorValues[value->DetailColorFunc], "uint8_t");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Detail Color Func type %x", c, value->DetailColorFunc);
        captainslog_warn("Unknown chunk: %s", (const char *)str);
        AddData(data, c, "Unknown", "uint8_t", (void *)&value->DetailColorFunc);
    }

    sprintf(c, "%s.DetailAlpha", name);

    if (value->DetailAlphaFunc < W3DSHADER_DETAILALPHAFUNC_MAX) {
        AddString(data, c, DetailAlphaValues[value->DetailAlphaFunc], "uint8_t");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Detail Alpha Func type %x", c, value->DetailAlphaFunc);
        captainslog_warn("Unknown chunk: %s", (const char *)str);
        AddData(data, c, "Unknown", "uint8_t", (void *)&value->DetailAlphaFunc);
    }

    sprintf(c, "%s.AlphaTest", name);

    if (value->AlphaTest < W3DSHADER_ALPHATEST_MAX) {
        AddString(data, c, AlphaTestValues[value->AlphaTest], "uint8_t");
    } else {
        StringClass str;
        str.Format("%s Shader unknown Alpha Test type %x", c, value->AlphaTest);
        captainslog_warn("Unknown chunk: %s", (const char *)str);
        AddData(data, c, "Unknown", "uint8_t", (void *)&value->AlphaTest);
    }
}

const char *PS2DepthCompareValues[] = { "Pass Never", "Pass Less", "Pass Always", "Pass Less or Equal" };

const char *PS2DepthMaskValues[] = { "Write Disable", "Write Enable", "Write Disable", "Write Enable" };

const char *PS2ABDParamValues[] = { "Src Color", "Dest Color", "Zero" };

const char *PS2CParamValues[] = {
    "Src Alpha", "Dest Alpha", "One", "Disable", "Enable", "Scale Fragment", "Replace Fragment"
};

const char *PS2PriGradientValues[] = { "Disable", "Modulate", "Highlight", "Highlight2", "Disable", "Enable" };

const char *PS2TexturingValues[] = { "Disable",
    "Enable",
    "Disable",
    "Detail",
    "Scale",
    "InvScale",
    "Add",
    "Sub",
    "SubR",
    "Blend",
    "DetailBlend",
    "Disable",
    "Detail",
    "Scale",
    "InvScale",
    "Disable",
    "Enable",
    "Smooth",
    "Flat" };

void AddPS2Shader(ChunkTreePtr &data, const char *name, const W3dPS2ShaderStruct *value)
{
    // main shader
    // TODO check if this is correct and works
    AddData(data, name, "W3dPS2ShaderStruct", "Main Shader", (void *)value);
    char c[256];
    sprintf(c, "%s.DepthCompare", name);
    AddData(data, c, "uint8_t", PS2DepthCompareValues[value->DepthCompare], (void *)&value->DepthCompare);
    AddString(data, c, PS2DepthCompareValues[value->DepthCompare], "uint8_t");
    sprintf(c, "%s.DepthMask", name);
    AddString(data, c, PS2DepthMaskValues[value->DepthMask], "uint8_t");
    sprintf(c, "%s.PriGradient", name);
    AddString(data, c, PS2PriGradientValues[value->PriGradient], "uint8_t");
    sprintf(c, "%s.Texturing", name);
    AddString(data, c, PS2TexturingValues[value->Texturing], "uint8_t");
    sprintf(c, "%s.AParam", name);
    AddString(data, c, PS2ABDParamValues[value->AParam], "uint8_t");
    sprintf(c, "%s.BParam", name);
    AddString(data, c, PS2ABDParamValues[value->BParam], "uint8_t");
    sprintf(c, "%s.CParam", name);
    AddString(data, c, PS2CParamValues[value->CParam], "uint8_t");
    sprintf(c, "%s.DParam", name);
    AddString(data, c, PS2ABDParamValues[value->DParam], "uint8_t");
}

void AddIJK(ChunkTreePtr &data, const char *name, const Vector3i *value)
{
    char c[256];
    sprintf(c, "%d %d %d", value->I, value->J, value->K);
    AddData(data, name, "Vector3i", c, (void *)value);
}

void AddIJK16(ChunkTreePtr &data, const char *name, const Vector3i16 *value)
{
    char c[256];
    sprintf(c, "%d %d %d", value->I, value->J, value->K);
    AddData(data, name, "Vector3i16", c, (void *)value);
}

// Macro for Defining Functions

#define DUMP_CHUNK(name, type) \
    void Dump_##name(ChunkLoadClass &cload, ChunkTreePtr &data) \
    { \
        char *chunkdata = ReadChunkData(cload); \
        type *chunkData = (type *)chunkdata; \
        AddData(data, #name, #type, "", (void *)chunkData); \
        delete[] chunkdata; \
    }

#define DUMP_CHUNK_ARRAY(name, type) \
    void Dump_##name(ChunkLoadClass &cload, ChunkTreePtr &data) \
    { \
        char *chunkdata = ReadChunkData(cload); \
        type *arrayData = (type *)chunkdata; \
        AddData(data, #name, #type "[]", "", (void *)arrayData); \
        delete[] chunkdata; \
    }

#define DUMP_CHUNK_PARSER(name) \
    void Dump_##name(ChunkLoadClass &cload, ChunkTreePtr &data) \
    { \
        AddString(data, #name, "", "string"); \
        ChunkManager::ParseSubchunks(cload, data->subchunks.back()); \
    }

#define SERIALIZE_CHUNK(name, type) \
    void Serialize_##name(ChunkSaveClass &csave, ChunkTreePtr &data) {}

#define SERIALIZE_CHUNK_ARRAY(name, type) \
    void Serialize_##name(ChunkSaveClass &csave, ChunkTreePtr &data) {}

#define SERIALIZE_CHUNK_PARSER(name) \
    void Serialize_##name(ChunkSaveClass &csave, ChunkTreePtr &data) {}

#define DUMP_SERIALIZE_CHUNK(name, type) \
    DUMP_CHUNK(name, type) \
    SERIALIZE_CHUNK(name, type)

#define DUMP_SERIALIZE_CHUNK_ARRAY(name, type) \
    DUMP_CHUNK_ARRAY(name, type) \
    SERIALIZE_CHUNK_ARRAY(name, type)

#define DUMP_SERIALIZE_PARSER(name) \
    DUMP_CHUNK_PARSER(name) \
    SERIALIZE_CHUNK_PARSER(name)

/*
Replacement:
void Dump_O_W3D_CHUNK_MATERIALS(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMaterialStruct *arrayData = (W3dMaterialStruct *)chunkdata;
    AddData(data,
        "O_W3D_CHUNK_MATERIALS", "W3dMaterialStruct" "[]", (void *)arrayData);
    delete[] chunkdata;
}
*/
DUMP_SERIALIZE_CHUNK_ARRAY(O_W3D_CHUNK_MATERIALS, W3dMaterialStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(O_W3D_CHUNK_MATERIALS2, W3dMaterial2Struct)
void Dump_O_W3D_C(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    // spilt name from Dump___func__
    const char *name = strstr(__func__, "Dump_") + 5;
    AddString(data, name, "", "string");
    data->subchunks = std::vector<ChunkTreePtr>(1);
    ChunkManager::ParseSubchunks(cload, data->subchunks.back());
}

void Dump_O_W3D_CHUNK_POV_QUADRANGLES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    AddString(data, "O_W3D_CHUNK_POV_QUADRANGLES", "unsupported", "string");
}

void Serialize_O_W3D_CHUNK_POV_QUADRANGLES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_O_W3D_CHUNK_POV_QUADRANGLES
}

void Dump_O_W3D_CHUNK_POV_TRIANGLES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    AddString(data, "O_W3D_CHUNK_POV_TRIANGLES", "unsupported", "string");
}

void Serialize_O_W3D_CHUNK_POV_TRIANGLES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_O_W3D_CHUNK_POV_QUADRANGLES
}

void Dump_O_W3D_CHUNK_QUADRANGLES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    AddString(data, "O_W3D_CHUNK_QUADRANGLES", "Outdated", "string");
}

void Serialize_O_W3D_CHUNK_QUADRANGLES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_O_W3D_CHUNK_POV_QUADRANGLES
}

DUMP_SERIALIZE_CHUNK_ARRAY(O_W3D_CHUNK_SURRENDER_TRIANGLES, W3dSurrenderTriangleStruct)

void Dump_O_W3D_CHUNK_TRIANGLES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    AddString(data, "O_W3D_CHUNK_TRIANGLES", "Obsolete", "string");
}

void Serialize_O_W3D_CHUNK_TRIANGLES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_O_W3D_CHUNK_TRIANGLES
}

DUMP_SERIALIZE_CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_COLOR_KEYFRAME, W3dEmitterColorKeyframeStruct)

DUMP_SERIALIZE_CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME, W3dEmitterOpacityKeyframeStruct)

DUMP_SERIALIZE_CHUNK(OBSOLETE_W3D_CHUNK_EMITTER_SIZE_KEYFRAME, W3dEmitterSizeKeyframeStruct)

DUMP_SERIALIZE_CHUNK(OBSOLETE_W3D_CHUNK_SHADOW_NODE, W3dHModelNodeStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_AABTREE)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_AABTREE_HEADER, W3dMeshAABTreeHeader)

void Dump_W3D_CHUNK_AABTREE_NODES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dMeshAABTreeNode *nodes = (W3dMeshAABTreeNode *)chunkdata;
    // TODO: valid
    for (unsigned int i = 0; i < cload.Cur_Chunk_Length() / sizeof(W3dMeshAABTreeNode); i++) {
        char c[256];
        sprintf(c, "Node[%d].Min", i);
        AddVector(data, c, &nodes[i].Min);
        sprintf(c, "Node[%d].Max", i);
        AddVector(data, c, &nodes[i].Max);

        if ((nodes[i].FrontOrPoly0 & 0x80000000) == 0) {
            sprintf(c, "Node[%d].Front", i);
            AddInt32(data, c, nodes[i].FrontOrPoly0);
            sprintf(c, "Node[%d].Back", i);
        } else {
            sprintf(c, "Node[%d].Poly0", i);
            AddInt32(data, c, nodes[i].FrontOrPoly0 & 0x7FFFFFFF);
            sprintf(c, "Node[%d].PolyCount", i);
        }

        AddInt32(data, c, nodes[i].BackOrPolyCount);
    }

    AddData(data, "W3D_CHUNK_AABTREE_NODES", "W3dMeshAABTreeNode[]", "", (void *)nodes);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_AABTREE_NODES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_AABTREE_NODES
}

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_AABTREE_POLYINDICES, uint32_t)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_AGGREGATE)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_AGGREGATE_CLASS_INFO, W3dAggregateMiscInfo)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_AGGREGATE_HEADER, W3dAggregateHeaderStruct)

void Dump_W3D_CHUNK_AGGREGATE_INFO(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dAggregateInfoStruct *info = (W3dAggregateInfoStruct *)chunkdata;
    AddString(data, "BaseModelName", info->BaseModelName, "string");
    AddInt32(data, "SubobjectCount", info->SubobjectCount);
    W3dAggregateSubobjectStruct *subobj = (W3dAggregateSubobjectStruct *)(chunkdata + sizeof(W3dAggregateInfoStruct));

    for (unsigned int i = 0; i < info->SubobjectCount; i++) {
        char c[256];
        sprintf(c, "SubObject[%u].SubobjectName", i);
        AddString(data, c, subobj[i].SubobjectName, "string");
        sprintf(c, "SubObject[%u].BoneName", i);
        AddString(data, c, subobj[i].BoneName, "string");
    }
    // TODO: valid?
    AddData(data, "W3D_CHUNK_AGGREGATE_INFO", "W3dAggregateInfoStruct", "", (void *)info);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_AGGREGATE_INFO(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_AGGREGATE_INFO
}

DUMP_SERIALIZE_PARSER(W3D_CHUNK_ANIMATION)

void Dump_W3D_CHUNK_ANIMATION_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dAnimChannelStruct *channel = (W3dAnimChannelStruct *)chunkdata;

    AddData(data, "W3D_CHUNK_ANIMATION_CHANNEL", "W3dAnimChannelStruct", "", (void *)channel);
    if (channel->Flags <= ANIM_CHANNEL_VIS) {

        StringClass str;
        str.Format("W3D_CHUNK_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_ANIMATION_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_ANIMATION_CHANNEL
}

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_ANIMATION_HEADER, W3dAnimHeaderStruct)

void Dump_W3D_CHUNK_BIT_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dBitChannelStruct *channel = (W3dBitChannelStruct *)chunkdata;
    AddData(data, "W3D_CHUNK_BIT_CHANNEL", "W3dBitChannelStruct", "", (void *)channel);
    if (channel->Flags <= BIT_CHANNEL_TIMECODED_VIS) {
        StringClass str;
        str.Format("W3D_CHUNK_BIT_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    delete[] chunkdata;
}
void Serialize_W3D_CHUNK_BIT_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_BIT_CHANNEL
}

void Dump_W3D_CHUNK_BOX(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dBoxStruct *box = (W3dBoxStruct *)chunkdata;
    AddData(data, "W3D_CHUNK_BOX", "W3dBoxStruct", "", (void *)box);

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

    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_BOX(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_BOX
}

DUMP_SERIALIZE_PARSER(W3D_CHUNK_COLLECTION)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_COLLECTION_HEADER, W3dCollectionHeaderStruct)

void Dump_W3D_CHUNK_COLLECTION_OBJ_NAME(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_COLLECTION_OBJ_NAME", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_COLLECTION_OBJ_NAME(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_COLLECTION_OBJ_NAME
}

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_COLLISION_NODE, W3dHModelNodeStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_DAMAGE)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_DAMAGE_COLORS, W3dDamageColorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_DAMAGE_HEADER, W3dDamageStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_DAMAGE_VERTICES, W3dDamageVertexStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_DAZZLE)

void Dump_W3D_CHUNK_DAZZLE_NAME(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_DAZZLE_NAME", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_DAZZLE_NAME(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_DAZZLE_NAME
}

void Dump_W3D_CHUNK_DAZZLE_TYPENAME(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_DAZZLE_TYPENAME", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_DAZZLE_TYPENAME(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_DAZZLE_TYPENAME
}

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_DCG, W3dRGBAStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_DIG, W3dRGBStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_EMITTER)

void Dump_W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterBlurTimeHeaderStruct *header = (W3dEmitterBlurTimeHeaderStruct *)chunkdata;
    AddInt32(data, "KeyframeCount", header->KeyframeCount);
    AddFloat(data, "Random", header->Random);
    W3dEmitterBlurTimeKeyframeStruct *blurtime =
        (W3dEmitterBlurTimeKeyframeStruct *)(chunkdata + sizeof(W3dEmitterBlurTimeHeaderStruct));

    for (unsigned int i = 0; i < header->KeyframeCount + 1; i++) {
        char c[256];
        sprintf(c, "Time[%u]", i);
        AddFloat(data, c, blurtime[i].Time);
        sprintf(c, "BlurTime[%u]", i);
        AddFloat(data, c, blurtime[i].BlurTime);
    }
    // TODO valid
    AddData(data, "W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES", "W3dEmitterBlurTimeKeyframeStruct[]", "", (void *)chunkdata);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES
}

void Dump_W3D_CHUNK_EMITTER_FRAME_KEYFRAMES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterFrameHeaderStruct *header = (W3dEmitterFrameHeaderStruct *)chunkdata;
    AddInt32(data, "KeyframeCount", header->KeyframeCount);
    AddFloat(data, "Random", header->Random);
    W3dEmitterFrameKeyframeStruct *frame =
        (W3dEmitterFrameKeyframeStruct *)(chunkdata + sizeof(W3dEmitterFrameHeaderStruct));

    for (unsigned int i = 0; i < header->KeyframeCount + 1; i++) {
        char c[256];
        sprintf(c, "Time[%u]", i);
        AddFloat(data, c, frame[i].Time);
        sprintf(c, "Frame[%u]", i);
        AddFloat(data, c, frame[i].Frame);
    }
    // TODO valid
    AddData(data, "W3D_CHUNK_EMITTER_FRAME_KEYFRAMES", "W3dEmitterFrameKeyframeStruct[]", "", (void *)chunkdata);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_EMITTER_FRAME_KEYFRAMES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_EMITTER_FRAME_KEYFRAMES
}

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_EMITTER_HEADER, W3dEmitterHeaderStruct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_EMITTER_INFO, W3dEmitterInfoStruct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_EMITTER_INFOV2, W3dEmitterInfoStructV2)

void Dump_W3D_CHUNK_EMITTER_PROPS(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterPropertyStruct *props = (W3dEmitterPropertyStruct *)chunkdata;
    AddInt32(data, "ColorKeyframes", props->ColorKeyframes);
    AddInt32(data, "OpacityKeyframes", props->OpacityKeyframes);
    AddInt32(data, "SizeKeyframes", props->SizeKeyframes);
    AddRGBA(data, "ColorRandom", &props->ColorRandom);
    AddFloat(data, "OpacityRandom", props->OpacityRandom);
    AddFloat(data, "SizeRandom", props->SizeRandom);
    W3dEmitterColorKeyframeStruct *color = (W3dEmitterColorKeyframeStruct *)(chunkdata + sizeof(W3dEmitterPropertyStruct));

    for (unsigned int i = 0; i < props->ColorKeyframes; i++) {
        char c[256];
        sprintf(c, "Time[%u]", i);
        AddFloat(data, c, color[i].Time);
        sprintf(c, "Color[%u]", i);
        AddRGBA(data, c, &color[i].Color);
    }

    W3dEmitterOpacityKeyframeStruct *opacity = (W3dEmitterOpacityKeyframeStruct *)(chunkdata
        + sizeof(W3dEmitterPropertyStruct) + (props->ColorKeyframes * sizeof(W3dEmitterColorKeyframeStruct)));

    for (unsigned int i = 0; i < props->OpacityKeyframes; i++) {
        char c[256];
        sprintf(c, "Time[%u]", i);
        AddFloat(data, c, opacity[i].Time);
        sprintf(c, "Opacity[%u]", i);
        AddFloat(data, c, opacity[i].Opacity);
    }

    W3dEmitterSizeKeyframeStruct *size = (W3dEmitterSizeKeyframeStruct *)(chunkdata + sizeof(W3dEmitterPropertyStruct)
        + (props->ColorKeyframes * sizeof(W3dEmitterColorKeyframeStruct))
        + (props->OpacityKeyframes * sizeof(W3dEmitterOpacityKeyframeStruct)));

    for (unsigned int i = 0; i < props->OpacityKeyframes; i++) {
        char c[256];
        sprintf(c, "Time[%u]", i);
        AddFloat(data, c, size[i].Time);
        sprintf(c, "Size[%u]", i);
        AddFloat(data, c, size[i].Size);
    }
    // TODO valid
    AddData(data, "W3D_CHUNK_EMITTER_PROPS", "W3dEmitterPropertyStruct", "", (void *)chunkdata);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_EMITTER_PROPS(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_EMITTER_PROPS
}

void Dump_W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dEmitterRotationHeaderStruct *header = (W3dEmitterRotationHeaderStruct *)chunkdata;
    AddInt32(data, "KeyframeCount", header->KeyframeCount);
    AddFloat(data, "Random", header->Random);
    AddFloat(data, "OrientationRandom", header->OrientationRandom);
    W3dEmitterRotationKeyframeStruct *frame =
        (W3dEmitterRotationKeyframeStruct *)(chunkdata + sizeof(W3dEmitterRotationHeaderStruct));

    for (unsigned int i = 0; i < header->KeyframeCount + 1; i++) {
        char c[256];
        sprintf(c, "Time[%u]", i);
        AddFloat(data, c, frame[i].Time);
        sprintf(c, "Rotation[%u]", i);
        AddFloat(data, c, frame[i].Rotation);
    }
    // TODO valid
    AddData(data, "W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES", "W3dEmitterRotationKeyframeStruct[]", "", (void *)chunkdata);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES
}

void Dump_W3D_CHUNK_EMITTER_USER_DATA(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_EMITTER_USER_DATA", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_EMITTER_USER_DATA(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_EMITTER_USER_DATA
}

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
    if (type == W3D_LIGHT_ATTRIBUTE_POINT) {
    } else if (type == W3D_LIGHT_ATTRIBUTE_SPOT) {
    } else if (type == W3D_LIGHT_ATTRIBUTE_DIRECTIONAL) {
    } else {
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
    // TODO: Implement Serialize_W3D_CHUNK_LIGHT_INFO
}

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_LIGHT_TRANSFORM, W3dLightTransformStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_LIGHTSCAPE)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_LIGHTSCAPE_LIGHT)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_LOD, W3dLODStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_LODMODEL)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_LODMODEL_HEADER, W3dLODModelHeaderStruct)

void Dump_W3D_CHUNK_MAP3_FILENAME(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_MAP3_FILENAME", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_MAP3_FILENAME(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_MAP3_FILENAME
}

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_MAP3_INFO, W3dMap3Struct)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_MATERIAL_INFO, W3dMaterialInfoStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MATERIAL_PASS)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MATERIAL3)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MATERIAL3_DC_MAP)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MATERIAL3_DI_MAP)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_MATERIAL3_INFO, W3dMaterial3Struct)

void Dump_W3D_CHUNK_MATERIAL3_NAME(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_MATERIAL3_NAME", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_MATERIAL3_NAME(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_MATERIAL3_NAME
}

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
    switch (type) {
        case W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL:
            break;
        case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED:
            break;
        case W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN:
            break;
        case OBSOLETE_W3D_MESH_FLAG_GEOMETRY_TYPE_SHADOW:
            break;
        case W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX:
            break;
        case W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX:
            break;
        case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED:
            break;
        case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_Z_ORIENTED:
            break;
        default: {
            StringClass str;
            str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Mesh Type %x", type);
            captainslog_warn((const char *)str);
            break;
        }
    }

    if (header->Attributes & 0x00000800) {
        StringClass str;
        str.Format("W3D_CHUNK_MESH_HEADER3 Unknown Attribute 0x00000800");
        captainslog_warn((const char *)str);
    }

    char c[64];

    if (header->Attributes & W3D_MESH_FLAG_PRELIT_MASK) {
        if (header->PrelitVersion) {
        } else {
            sprintf(c, "UNKNOWN");
        }
    } else {
        sprintf(c, "N/A");
    }
    //    AddString(data, "PrelitVersion", c, "string");
    // TODO valid
    captainslog_warn((const char *)"str");

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
    // TODO: Implement Serialize_W3D_CHUNK_MESH_HEADER3
}

void Dump_W3D_CHUNK_MESH_USER_TEXT(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_MESH_USER_TEXT", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_MESH_USER_TEXT(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_MESH_USER_TEXT
}

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

    if (hint == W3DTEXTURE_HINT_BASE) {
    } else if (hint == W3DTEXTURE_HINT_EMISSIVE) {
    } else if (hint == W3DTEXTURE_HINT_ENVIRONMENT) {
    } else if (hint == W3DTEXTURE_HINT_SHINY_MASK) {
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_TEXTURE_INFO Unknown Hints %x", hint);
        captainslog_warn((const char *)str);
    }

    if (info->Attributes & 0xE000) {
        StringClass str;
        str.Format("W3D_CHUNK_TEXTURE_INFO Unknown Flags %x", info->Attributes & 0xE000);
        captainslog_warn((const char *)str);
    }

    if (info->AnimType == W3DTEXTURE_ANIM_LOOP) {
    } else if (info->AnimType == W3DTEXTURE_ANIM_PINGPONG) {
    } else if (info->AnimType == W3DTEXTURE_ANIM_ONCE) {
    } else if (info->AnimType == W3DTEXTURE_ANIM_MANUAL) {
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_TEXTURE_INFO Unknown Anim Type %x", info->AnimType);
        captainslog_warn((const char *)str);
    }

    AddData(data, "W3D_CHUNK_TEXTURE_INFO", "W3dTextureInfoStruct", "", (void *)info);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_TEXTURE_INFO(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_TEXTURE_INFO
}

void Dump_W3D_CHUNK_TEXTURE_NAME(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_TEXTURE_NAME", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_TEXTURE_NAME(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_TEXTURE_NAME
}

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_TEXTURE_REPLACER_INFO, W3dTextureReplacerStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_TEXTURE_STAGE)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_TEXTURES)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_TRANSFORM_NODE, W3dPlaceholderStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_TRIANGLES, W3dTriStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_VERTEX_COLORS, W3dRGBStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_VERTEX_INFLUENCES, W3dVertInfStruct)

void Dump_W3D_CHUNK_VERTEX_MAPPER_ARGS0(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_VERTEX_MAPPER_ARGS0", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_VERTEX_MAPPER_ARGS0(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_VERTEX_MAPPER_ARGS0
}

void Dump_W3D_CHUNK_VERTEX_MAPPER_ARGS1(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_VERTEX_MAPPER_ARGS1", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_VERTEX_MAPPER_ARGS1(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_VERTEX_MAPPER_ARGS1
}

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

    if ((material->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_UV) {
        AddString(data, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_UV", "flag");
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
    // TODO: Implement Serialize_W3D_CHUNK_VERTEX_MATERIAL_INFO
}

void Dump_W3D_CHUNK_VERTEX_MATERIAL_NAME(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_VERTEX_MATERIAL_NAME", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_VERTEX_MATERIAL_NAME(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_VERTEX_MATERIAL_NAME
}

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

    int mapmode = props->Flags >> W3D_ELINE_TEXTURE_MAP_MODE_OFFSET;

    switch (mapmode) {
        case W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP:
            break;
        case W3D_ELINE_UNIFORM_LENGTH_TEXTURE_MAP:
            break;
        case W3D_ELINE_TILED_TEXTURE_MAP:
            break;
        default: {
            StringClass str;
            str.Format("W3D_CHUNK_EMITTER_LINE_PROPERTIES Unknown Emitter Mapping Mode %x", mapmode);
            captainslog_warn((const char *)str);
            break;
        }
    }
    AddData(data, "W3D_CHUNK_EMITTER_LINE_PROPERTIES", "W3dEmitterLinePropertiesStruct", "", (void *)props);
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_EMITTER_LINE_PROPERTIES(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_EMITTER_LINE_PROPERTIES
}

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SECONDARY_VERTICES, W3dVectorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_SECONDARY_VERTEX_NORMALS, W3dVectorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_TANGENTS, W3dVectorStruct)

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_BINORMALS, W3dVectorStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_COMPRESSED_ANIMATION)

int flavor = ANIM_FLAVOR_TIMECODED;
const char *FlavorTypes[] = { "Timecoded", "Adaptive Delta" };

void Dump_W3D_CHUNK_COMPRESSED_ANIMATION_HEADER(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dCompressedAnimHeaderStruct *header = (W3dCompressedAnimHeaderStruct *)chunkdata;
    if (header->Flavor < ANIM_FLAVOR_VALID) {
    } else {
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
    // TODO: Implement Serialize_W3D_CHUNK_COMPRESSED_ANIMATION_HEADER
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

        delete[] chunkdata;
    } else {
        W3dAdaptiveDeltaAnimChannelStruct *channel = (W3dAdaptiveDeltaAnimChannelStruct *)chunkdata;

        if (channel->Flags <= ANIM_CHANNEL_VIS) {
        } else {
            StringClass str;
            str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
            captainslog_warn((const char *)str);
        }

        delete[] chunkdata;
    }
    // TODO valid
    //    AddData(data, "W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL", "char*", "", (void *)channel);
}

void Serialize_W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL
}

void Dump_W3D_CHUNK_COMPRESSED_BIT_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dTimeCodedBitChannelStruct *channel = (W3dTimeCodedBitChannelStruct *)chunkdata;

    if (channel->Flags <= BIT_CHANNEL_TIMECODED_VIS) {
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_BIT_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }
    AddData(data, "W3D_CHUNK_COMPRESSED_BIT_CHANNEL", "W3dTimeCodedBitChannelStruct", "", (void *)channel);

    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_COMPRESSED_BIT_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_COMPRESSED_BIT_CHANNEL
}

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MORPH_ANIMATION)

DUMP_SERIALIZE_CHUNK(W3D_CHUNK_MORPHANIM_HEADER, W3dMorphAnimHeaderStruct)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_MORPHANIM_CHANNEL)

void Dump_W3D_CHUNK_MORPHANIM_POSENAME(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_MORPHANIM_POSENAME", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_MORPHANIM_POSENAME(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_MORPHANIM_POSENAME
}

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

void Serialize_DoVector3Channel(ChunkSaveClass &csave, ChunkTreePtr &data, const char *name)
{
    // TODO: Implement Serialize_DoVector3Channel
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

void Serialize_DoVector2Channel(ChunkSaveClass &csave, ChunkTreePtr &data, const char *name)
{
    // TODO: Implement Serialize_DoVector2Channel
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

void Serialize_DofloatChannel(ChunkSaveClass &csave, ChunkTreePtr &data, const char *name)
{
    // TODO: Implement Serialize_DofloatChannel
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

void Serialize_DoAlphaVectorStructChannel(ChunkSaveClass &csave, ChunkTreePtr &data, const char *name)
{
    // TODO: Implement Serialize_DoAlphaVectorStructChannel
}

void Dump_W3D_CHUNK_RING(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    while (cload.Open_Chunk()) {
        switch (cload.Cur_Chunk_ID()) {
            case 1: {
                W3dRingStruct RingStruct;
                cload.Read(&RingStruct, sizeof(RingStruct));
                AddInt32(data, "unk0", RingStruct.unk0);
                AddInt32(data, "Flags", RingStruct.Flags);

                if (RingStruct.Flags & RING_CAMERA_ALIGNED) {
                    AddString(data, "Flags", "RING_CAMERA_ALIGNED", "flag");
                }

                if (RingStruct.Flags & RING_LOOPING) {
                    AddString(data, "Flags", "RING_LOOPING", "flag");
                }

                if (RingStruct.Flags & 0xFFFFFFFC) {
                    StringClass str;
                    str.Format("W3D_CHUNK_RING Unknown Ring Flags %x", RingStruct.Flags & 0xFFFFFFFC);
                    captainslog_warn((const char *)str);
                }

                AddString(data, "Name", RingStruct.Name, "string");
                AddVector(data, "Center", &RingStruct.Center);
                AddVector(data, "Extent", &RingStruct.Extent);
                AddFloat(data, "AnimationDuration", RingStruct.AnimationDuration);
                AddVector(data, "Color", &RingStruct.Color);
                AddFloat(data, "Alpha", RingStruct.Alpha);
                AddFloat(data, "InnerScale.X", RingStruct.InnerScale.x);
                AddFloat(data, "InnerScale.Y", RingStruct.InnerScale.y);
                AddFloat(data, "OuterScale.X", RingStruct.OuterScale.x);
                AddFloat(data, "OuterScale.Y", RingStruct.OuterScale.y);
                AddFloat(data, "InnerExtent.X", RingStruct.InnerExtent.x);
                AddFloat(data, "InnerExtent.Y", RingStruct.InnerExtent.y);
                AddFloat(data, "OuterExtent.X", RingStruct.OuterExtent.x);
                AddFloat(data, "OuterExtent.Y", RingStruct.OuterExtent.y);
                AddString(data, "TextureName", RingStruct.TextureName, "string");
                AddShader(data, "Shader", &RingStruct.Shader);
                AddInt32(data, "TextureTiling", RingStruct.TextureTiling);
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
}

void Dump_W3D_CHUNK_SPHERE(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    while (cload.Open_Chunk()) {
        switch (cload.Cur_Chunk_ID()) {
            case 1: {
                W3dSphereStruct SphereStruct;
                cload.Read(&SphereStruct, sizeof(SphereStruct));
                AddInt32(data, "unk0", SphereStruct.unk0);
                AddInt32(data, "Flags", SphereStruct.Flags);
                if (SphereStruct.Flags & SPHERE_ALPHA_VECTOR) {
                    AddString(data, "Flags", "SPHERE_ALPHA_VECTOR", "flag");
                }
                if (SphereStruct.Flags & SPHERE_CAMERA_ALIGNED) {
                    AddString(data, "Flags", "SPHERE_CAMERA_ALIGNED", "flag");
                }
                if (SphereStruct.Flags & SPHERE_INVERT_EFFECT) {
                    AddString(data, "Flags", "SPHERE_INVERT_EFFECT", "flag");
                }
                if (SphereStruct.Flags & SPHERE_LOOPING) {
                    AddString(data, "Flags", "SPHERE_LOOPING", "flag");
                }
                if (SphereStruct.Flags & 0xFFFFFFF0) {
                    StringClass str;
                    str.Format("W3D_CHUNK_SPHERE Unknown Sphere Flags %x", SphereStruct.Flags & 0xFFFFFFFC);
                    captainslog_warn((const char *)str);
                }
                AddString(data, "Name", SphereStruct.Name, "string");
                AddVector(data, "Center", &SphereStruct.Center);
                AddVector(data, "Extent", &SphereStruct.Extent);
                AddFloat(data, "AnimationDuration", SphereStruct.AnimationDuration);
                AddVector(data, "Color", &SphereStruct.Color);
                AddFloat(data, "Alpha", SphereStruct.Alpha);
                AddVector(data, "Scale", &SphereStruct.Scale);
                AddFloat(data, "Vector.Quat.X", SphereStruct.Vector.Quat.X);
                AddFloat(data, "Vector.Quat.Y", SphereStruct.Vector.Quat.Y);
                AddFloat(data, "Vector.Quat.Z", SphereStruct.Vector.Quat.Z);
                AddFloat(data, "Vector.Quat.W", SphereStruct.Vector.Quat.W);
                AddFloat(data, "Vector.Magnutide", SphereStruct.Vector.Magnitude);
                AddString(data, "TextureName", SphereStruct.TextureName, "string");
                AddShader(data, "Shader", &SphereStruct.Shader);
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
}

DUMP_SERIALIZE_PARSER(W3D_CHUNK_SHDMESH)

void Dump_W3D_CHUNK_SHDMESH_NAME(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_SHDMESH_NAME", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_SHDMESH_NAME(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_SHDMESH_NAME
}

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

void Dump_W3D_CHUNK_SHDMESH_USER_TEXT(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    AddString(data, "W3D_CHUNK_SHDMESH_USER_TEXT", chunkdata, "string");
    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_SHDMESH_USER_TEXT(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_SHDMESH_USER_TEXT
}

DUMP_SERIALIZE_CHUNK_ARRAY(W3D_CHUNK_FXSHADER_IDS, uint32_t)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_FX_SHADERS)

DUMP_SERIALIZE_PARSER(W3D_CHUNK_FX_SHADER)

void Dump_W3D_CHUNK_FX_SHADER_INFO(ChunkLoadClass &cload, ChunkTreePtr &data)
{
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
}

const char *Types[] = { "Texture", "Float", "Vector2", "Vector3", "Vector4", "Int", "Bool" };

void Dump_W3D_CHUNK_FX_SHADER_CONSTANT(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    uint32_t type = *(uint32_t *)chunkdata;
    uint32_t length = *(uint32_t *)(chunkdata + 4);
    char *constantname = (char *)(chunkdata + 4 + 4);
    AddString(data, "Type", Types[type - 1], "String");
    AddString(data, "Constant Name", constantname, "String");

    if (type == CONSTANT_TYPE_TEXTURE) {
        char *texture = (char *)(chunkdata + 4 + 4 + length + 4);
        AddString(data, "Texture", texture, "String");
    } else if (type >= CONSTANT_TYPE_FLOAT1 && type <= CONSTANT_TYPE_FLOAT4) {
        int count = type - 1;
        float *floats = (float *)(chunkdata + 4 + 4 + length);
        AddFloatArray(data, "Floats", floats, count);
    } else if (type == CONSTANT_TYPE_INT) {
        uint32_t u = *(uint32_t *)(chunkdata + 4 + 4 + length);
        AddInt32(data, "Int", u);
    } else if (type == CONSTANT_TYPE_BOOL) {
        uint8_t u = *(uint8_t *)(chunkdata + 4 + 4 + length);
        AddInt32(data, "Bool", u);
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_FX_SHADER_CONSTANT Unknown Constant Type %x", type);
        captainslog_warn((const char *)str);
    }

    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_FX_SHADER_CONSTANT(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_FX_SHADER_CONSTANT
}
const char *NewFlavorTypes[] = { "Timecoded", "Adaptive Delta 4", "Adaptive Delta 8" };

void Dump_W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL(ChunkLoadClass &cload, ChunkTreePtr &data)
{
    char *chunkdata = ReadChunkData(cload);
    W3dCompressedMotionChannelStruct *channel = (W3dCompressedMotionChannelStruct *)chunkdata;

    if (channel->Flavor < ANIM_FLAVOR_NEW_VALID) {
        AddString(data, "Flavor", NewFlavorTypes[channel->Flavor], "string");
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL Unknown Flavor Type %x", channel->Flavor);
        captainslog_warn((const char *)str);
    }

    AddInt8(data, "VectorLen", channel->VectorLen);

    if (channel->Flags <= ANIM_CHANNEL_VIS) {
        //        AddString(data, "ChannelType", ChannelTypes[channel->Flags], "string");
    } else {
        StringClass str;
        str.Format("W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL Unknown Animation Channel Type %x", channel->Flags);
        captainslog_warn((const char *)str);
    }

    AddInt32(data, "NumTimeCodes", channel->NumTimeCodes);
    AddInt16(data, "Pivot", channel->Pivot);

    if (channel->Flavor == ANIM_FLAVOR_NEW_TIMECODED) {
        uint16_t *keyframes = (uint16_t *)(chunkdata + sizeof(W3dCompressedMotionChannelStruct));

        for (int i = 0; i < channel->NumTimeCodes; i++) {
            StringClass str;
            str.Format("KeyFrames[%d]", i);
            AddInt32(data, str, keyframes[i]);
        }

        int datalen = channel->VectorLen * channel->NumTimeCodes;
        int pos = sizeof(W3dCompressedMotionChannelStruct);
        pos += channel->NumTimeCodes * 2;

        if (channel->NumTimeCodes & 1) {
            pos += 2;
        }

        uint32_t *values = (uint32_t *)(chunkdata + pos);

        for (int i = 0; i < datalen; i++) {
            StringClass str;
            str.Format("Data[%d]", i);
            AddInt32(data, str, values[i]);
        }
    } else {
        float *scale = (float *)(chunkdata + sizeof(W3dCompressedMotionChannelStruct));
        AddFloat(data, "Scale", *scale);
        float *initial = (float *)(chunkdata + sizeof(W3dCompressedMotionChannelStruct) + 4);

        for (int i = 0; i < channel->VectorLen; i++) {
            StringClass str;
            str.Format("Initial[%d]", i);
            AddFloat(data, str, initial[i]);
        }

        int count = (cload.Cur_Chunk_Length() - sizeof(W3dCompressedMotionChannelStruct) - 4 - 4 * channel->VectorLen) / 4;
        uint32_t *values = (uint32_t *)(chunkdata + sizeof(W3dCompressedMotionChannelStruct) + 4 + 4 * channel->VectorLen);

        for (int i = 0; i < count; i++) {
            StringClass str;
            str.Format("Data[%d]", i);
            AddInt32(data, str, values[i]);
        }
    }

    delete[] chunkdata;
}

void Serialize_W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL(ChunkSaveClass &csave, ChunkTreePtr &data)
{
    // TODO: Implement Serialize_W3D_CHUNK_COMPRESSED_ANIMATION_MOTION_CHANNEL
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