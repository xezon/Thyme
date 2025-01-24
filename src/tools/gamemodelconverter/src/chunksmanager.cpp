#include "chunksmanager.h"

ChunkManager::ChunkManager(const char *filePath, int flag)
{
    m_rootChunk = std::make_unique<ChunkTree>();

//    if (flag == LOAD) {
//        // Open the file for reading
//        m_file = auto_file_ptr(g_theFileFactory, filePath); // Assuming auto_file_ptr is a smart pointer
//        if (!m_file->Open(FileOpenType::FM_READ)) {
//            captainslog_error("Could not open file for reading: %s", filePath);
//        }
//    } else if (flag == SAVE) {
//        // Open the file for writing
//        m_file = auto_file_ptr(g_theFileFactory, filePath); // Assuming auto_file_ptr is a smart pointer
//        if (!m_file->Open(FileOpenType::FM_WRITE)) {
//            captainslog_error("Could not open file for writing: %s", filePath);
//        }
//    } else {
//        captainslog_error("Invalid flag in ChunkManager constructor");
//    }
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
        captainslog_error("Parent chunk is null");
        return;
    }
    while (chunkLoader.Open_Chunk()) {
        ChunkPtr chunk = std::make_unique<Chunk>();
        ReadChunkInfo(chunkLoader, chunk);
        parentChunk->data = std::move(chunk);
        if (chunkLoader.Contains_Chunks()) {
            while (chunkLoader.Open_Chunk()) {
                ChunkPtr subchunk = std::make_unique<Chunk>();
                ReadChunkInfo(chunkLoader, subchunk);
                parentChunk->subchunks.push_back(std::move(subchunk));
                chunkLoader.Close_Chunk();
            }
        }
        chunkLoader.Close_Chunk();
    }
}

std::map<int, ChunkIOFuncs> typeToIOFuncMap;

void ChunkManager::ReadChunkInfo(ChunkLoadClass &chunkLoader, ChunkPtr &chunk)
{
    if (chunk == nullptr) {
        captainslog_error("Chunk is null");
        return;
    }
    chunk->chunkType = chunkLoader.Cur_Chunk_ID();
    chunk->chunkSize = chunkLoader.Cur_Chunk_Length();
    auto it = typeToIOFuncMap.find(chunk->chunkType);
    if (it != typeToIOFuncMap.end()) {
        chunk->info = createChunkInfo();
        it->second.ReadFunc(chunkLoader, chunk->info);
    } else {
        StringClass str;
        str.Format("Unknown Chunk 0x%X", chunk->chunkType);
        captainslog_warn((const char *)str);
        chunk->info = nullptr;
    }
}

void ChunkManager::WriteSubchunks(ChunkSaveClass &chunkSaver, const ChunkTreePtr &parentChunk)
{
    if (parentChunk->data) {
        if (!chunkSaver.Begin_Chunk(parentChunk->data->chunkType)) {
            captainslog_error("Failed to begin chunk");
            return;
        }
        WriteChunkInfo(chunkSaver, parentChunk->data);
        if (!chunkSaver.End_Chunk()) {
            captainslog_error("Failed to end chunk");
            return;
        }

    }

    for (const auto& subchunk : parentChunk->subchunks) {
        WriteChunkInfo(chunkSaver, subchunk);
    }
}

void ChunkManager::WriteChunkInfo(ChunkSaveClass &chunkSaver, const ChunkPtr &chunk)
{
    auto it = typeToIOFuncMap.find(chunk->chunkType);
    if (it != typeToIOFuncMap.end()) {
        it->second.WriteFunc(chunkSaver, chunk->info);
    } else {
        StringClass str;
        str.Format("Unknown Chunk 0x%X", chunk->chunkType);
        captainslog_warn((const char*)str);
    }
}

void ChunkManager::AddData(
    ChunkInfoPtr &data, const char *name, const StringClass &type, const StringClass &formattedValue, void *value)
{
    if (data == nullptr) {
        data = std::move(createChunkInfo(name, type, formattedValue, value));
    } else {
        captainslog_error("ChunkInfo %s already exists, cannot set %s", (const char*)data->name, name);
    }
    data = std::move(createChunkInfo(name, type, formattedValue, value));
}

void ChunkManager::AddString(ChunkInfoPtr &data, const char *name, const StringClass &value, const StringClass &type)
{
    AddData(data, name, type, value, nullptr);
}

void ChunkManager::AddVersion(ChunkInfoPtr &data, uint32_t value)
{
    char c[64];
    sprintf(c, "%u.%hu", value >> 16, (uint16_t)value);
    AddData(data, "Version", "uint32_t", c, (void *)&value);
}

void ChunkManager::AddInt32(ChunkInfoPtr &data, const char *name, uint32_t value)
{
    char c[256];
    sprintf(c, "%u", value);
    AddData(data, name, "uint32_t", c, (void *)&value);
}

void ChunkManager::AddInt16(ChunkInfoPtr &data, const char *name, uint16_t value)
{
    char c[256];
    sprintf(c, "%hu", value);
    AddData(data, name, "uint16_t", c, (void *)&value);
}

void ChunkManager::AddInt8(ChunkInfoPtr &data, const char *name, uint8_t value)
{
    char c[256];
    sprintf(c, "%hhu", value);
    AddData(data, name, "uint8_t", c, (void *)&value);
}

void ChunkManager::AddInt8Array(ChunkInfoPtr &data, const char *name, const uint8_t *values, int count)
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

void ChunkManager::AddFloat(ChunkInfoPtr &data, const char *name, float value)
{
    char c[256];
    sprintf(c, "%f", value);
    AddData(data, name, "float", c, (void *)&value);
}

void ChunkManager::AddInt32Array(ChunkInfoPtr &data, const char *name, const uint32_t *values, int count)
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

void ChunkManager::AddFloatArray(ChunkInfoPtr &data, const char *name, const float *values, int count)
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

void ChunkManager::AddVector(ChunkInfoPtr &data, const char *name, const W3dVectorStruct *value)
{
    char c[256];
    sprintf(c, "%f %f %f", value->x, value->y, value->z);
    AddData(data, name, "W3dVectorStruct", c, (void *)value);
}

void ChunkManager::AddQuaternion(ChunkInfoPtr &data, const char *name, const W3dQuaternionStruct *value)
{
    char c[256];
    sprintf(c, "%f %f %f %f", value->q[0], value->q[1], value->q[2], value->q[3]);
    AddData(data, name, "W3dQuaternionStruct", c, (void *)value);
}

void ChunkManager::AddRGB(ChunkInfoPtr &data, const char *name, const W3dRGBStruct *value)
{
    StringClass str;
    str.Format("(%hhu %hhu %hhu) ", value->r, value->g, value->b);
    AddData(data, name, "W3dRGBStruct", str, (void *)value);
}

void ChunkManager::AddRGBArray(ChunkInfoPtr &data, const char *name, const W3dRGBStruct *values, int count)
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

void ChunkManager::AddRGBA(ChunkInfoPtr &data, const char *name, const W3dRGBAStruct *value)
{
    StringClass str;
    str.Format("(%hhu %hhu %hhu %hhu) ", value->R, value->G, value->B, value->A);
    AddData(data, name, "W3dRGBAStruct", str, (void *)value);
}

void ChunkManager::AddTexCoord(ChunkInfoPtr &data, const char *name, const W3dTexCoordStruct *value)
{
    char c[256];
    sprintf(c, "%f %f", value->U, value->V);
    AddData(data, name, "W3dTexCoordStruct", c, (void *)value);
}

void ChunkManager::AddTexCoordArray(ChunkInfoPtr &data, const char *name, const W3dTexCoordStruct *values, int count)
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

void ChunkManager::AddShader(ChunkInfoPtr &data, const char *name, const W3dShaderStruct *value)
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

void ChunkManager::AddPS2Shader(ChunkInfoPtr &data, const char *name, const W3dPS2ShaderStruct *value)
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

void ChunkManager::AddIJK(ChunkInfoPtr &data, const char *name, const Vector3i *value)
{
    char c[256];
    sprintf(c, "%d %d %d", value->I, value->J, value->K);
    AddData(data, name, "Vector3i", c, (void *)value);
}

void ChunkManager::AddIJK16(ChunkInfoPtr &data, const char *name, const Vector3i16 *value)
{
    char c[256];
    sprintf(c, "%d %d %d", value->I, value->J, value->K);
    AddData(data, name, "Vector3i16", c, (void *)value);
}
