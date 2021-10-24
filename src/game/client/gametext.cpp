/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief String file handler.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gametext.h"
#include "filesystem.h"
#include "main.h" // For g_applicationHWnd
#include "registry.h"
#include "rtsutils.h"
#include <algorithm>
#include <captainslog.h>

#ifdef PLATFORM_WINDOWS
#include <winuser.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef GAME_DLL
#include "hookcrt.h"
#endif

using rts::FourCC;
using rts::FourCC_LE;

#ifndef GAME_DLL
GameTextInterface *g_theGameText = nullptr;
#endif

// Comparison function used for sorting and searching StringLookUp arrays.
int GameTextManager::Compare_LUT(void const *a, void const *b)
{
    const char *ac = static_cast<StringLookUp const *>(a)->label->Str();
    const char *bc = static_cast<StringLookUp const *>(b)->label->Str();

    return strcasecmp(ac, bc);
}

GameTextInterface *GameTextManager::Create_Game_Text_Interface()
{
    return new GameTextManager;
}

// Get a char from a file.
char GameTextFile::Read_Char(File *file)
{
    char tmp;

    if (file->Read(&tmp, sizeof(tmp)) == sizeof(tmp)) {
        return tmp;
    }

    return '\0';
}

char GameTextManager::Read_Char(File *file)
{
    return GameTextFile::Read_Char(file);
}

// Read a quoted string and also any extra data.
void GameTextFile::Read_To_End_Of_Quote(File *file, char *in, char *out, char *wave, int buff_len)
{
    bool escape = false;
    int i;

    for (i = 0; i < buff_len; ++i) {
        char current;

        // if in pointer is valid, read data from that, otherwise read from file.
        if (in != nullptr) {
            current = *in++;

            if (current == '\0') {
                in = nullptr;
                current = Read_Char(file);
            }
        } else {
            current = Read_Char(file);
        }

        // -1 and we are done?
        if (current == '\xFF') {
            return;
        }

        // Handle some special characters.
        if (current == '\n') {
            escape = false;
            current = ' ';
        } else if (current == '\\') {
            escape = !escape;
        } else if (current == '"' && !escape) {
            break;
        } else {
            escape = false;
        }

        // Treat any white space as a space char.
        if (isspace(uint8_t(current))) {
            current = ' ';
        }

        // Copy to output buffer.
        out[i] = current;
    }

    out[i] = '\0';

    int wave_pos = 0;
    int state = 0;

    while (true) {
        char current;

        // If in pointer is valid, read data from that, otherwise read from file.
        if (in != nullptr) {
            current = *in++;

            if (current == '\0') {
                in = nullptr;
                current = Read_Char(file);
            }
        } else {
            current = Read_Char(file);
        }

        // Stop reading on line break or -1 char.
        if (current == '\n' || current == '\xFF') {
            break;
        }

        // state 0 ignores initial whitespace and '='
        if (state == 0 && !isspace(uint8_t(current)) && current != '=') {
            state = 1;
        }

        // state 1 read so long as its alphanumeric characters or underscore.
        if (state == 1) {
            if ((current < 'a' || current > 'z') && (current < 'A' || current > 'Z') && (current < '0' || current > '9')
                && current != '_') {
                state = 2;
            } else {
                wave[wave_pos++] = current;
            }
        }

        // state 2 ignore everything and keep reading until a breaking condition is encountered.
    }

    if (wave_pos > 0) {
        if (wave[wave_pos - 1] >= '0' && wave[wave_pos - 1] <= '9') {
            wave[wave_pos++] = 'e';
        }
    }

    wave[wave_pos] = '\0';
}

void GameTextManager::Read_To_End_Of_Quote(File *file, char *in, char *out, char *wave, int buff_len)
{
    GameTextFile::Read_To_End_Of_Quote(file, in, out, wave, buff_len);
}

// Read a line from a str file into provided buffer.
bool GameTextFile::Read_Line(char *buffer, int length, File *file)
{
    bool ret = false;
    char *putp = buffer;

    for (int i = 0; i < length; ++i) {
        if (file->Read(putp, sizeof(*putp)) != sizeof(*putp)) {
            break;
        }

        ret = true;

        if (*putp == '\n') {
            break;
        }

        ++putp;
    }

    *putp = '\0';

    return ret;
}

bool GameTextManager::Read_Line(char *buffer, int length, File *file)
{
    return GameTextFile::Read_Line(buffer, length, file);
}

// Converts an ASCII string into a usc2/utf16 string.
void GameTextFile::Translate_Copy(unichar_t *out, const char *in)
{
    // Unsigned allows handling Latin extended code page.
    const unsigned char *in_unsigned = reinterpret_cast<const unsigned char *>(in);
    unsigned char current = *in_unsigned++;
    bool escape = false;

    while (current != '\0') {
        // Last char was a '\', handle for escape sequence.
        if (escape) {
            escape = false;

            switch (current) {
                case '\\':
                    *out++ = U_CHAR('\\');
                    break;
                case '\'':
                    *out++ = U_CHAR('\'');
                    break;
                case '"':
                    *out++ = U_CHAR('"');
                    break;
                case '?':
                    *out++ = U_CHAR('?');
                    break;
                case 't':
                    *out++ = U_CHAR('\t');
                    break;
                case 'n':
                    *out++ = U_CHAR('\n');
                    break;
                case '\0':
                    return;
                default:
                    *out++ = current;
                    break;
            }
        } else {
            if (current == '\\') {
                // Set to handle escape sequence.
                escape = true;
            } else {
                // Otherwise its a naive copy, assumes input is pure ascii.
                *out++ = current;
            }
        }

        current = *in_unsigned++;
    }

    // Null terminate.
    *out = U_CHAR('\0');
}

void GameTextManager::Translate_Copy(unichar_t *out, char *in)
{
    GameTextFile::Translate_Copy(out, in);
}

// Remove whitespace from the start and end of a ascii/utf8 string.
void GameTextFile::Remove_Leading_And_Trailing(char *buffer)
{
    int first = 0;

    // Find first none whitespace char.
    while (buffer[first] != '\0' && isspace(uint8_t(buffer[first]))) {
        ++first;
    }

    int pos = 0;

    // Move data down to start of buffer.
    while (buffer[first] != '\0') {
        buffer[pos++] = buffer[first++];
    }

    // Move pos back to last none whitespace.
    while (--pos >= 0 && isspace(buffer[pos])) {
        // Empty loop.
    }

    // Null terminate after last none whitespace.
    buffer[pos + 1] = '\0';
}

void GameTextManager::Remove_Leading_And_Trailing(char *buffer)
{
    GameTextFile::Remove_Leading_And_Trailing(buffer);
}

// Strip spaces from a ucs2/utf16 string.
void GameTextFile::Strip_Spaces(unichar_t *buffer)
{
    unichar_t *getp = buffer;
    unichar_t *putp = buffer;

    unichar_t current = *getp++;
    unichar_t last = U_CHAR('\0');

    bool prev_whitepsace = true;

    while (current != U_CHAR('\0')) {
        if (current == U_CHAR(' ')) {
            if (last == U_CHAR(' ') || prev_whitepsace) {
                current = *getp++;

                continue;
            }
        } else if (current == U_CHAR('\n') || current == U_CHAR('\t')) {
            if (last == U_CHAR(' ')) {
                --putp;
            }

            *putp++ = current;
            prev_whitepsace = true;
            last = current;
            current = *getp++;

            continue;
        }

        *putp++ = current;
        prev_whitepsace = false;
        last = current;
        current = *getp++;
    }

    if (last == U_CHAR(' ')) {
        --putp;
    }

    // Ensure we null terminate after the last shuffled character.
    *putp = U_CHAR('\0');
}

void GameTextManager::Strip_Spaces(unichar_t *buffer)
{
    GameTextFile::Strip_Spaces(buffer);
}

// Reverses a word, appears to be unused in release, involved in jabberwocky setting
// in old dev builds its seems
void GameTextManager::Reverse_Word(char *start, char *end)
{
    bool first_char = true;

    while (start < end) {
        char s = *start;
        char e = *end;

        // Swap the capitalization for first char if its within standard alphabet.
        if (first_char) {
            if (s >= 'A' && s <= 'Z' && e >= 'a' && e <= 'z') {
                s += 32;
                e -= 32;
            }

            first_char = false;
        }

        *start++ = e;
        *end-- = s;
    }
}

// Get the count of strings in a str file.
bool GameTextFile::Get_String_Count(
    const char *filename, int &count, BufferView<char> buffer_in, BufferView<char> buffer_out, BufferView<char> buffer_ex)
{
    File *file = g_theFileSystem->Open(filename, File::TEXT | File::READ);
    count = 0;

    if (file == nullptr) {
        return false;
    }

    while (Read_Line(buffer_in, buffer_in.Size() - 1, file)) {
        Remove_Leading_And_Trailing(buffer_in);

        if (buffer_in[0] == '"') {
            size_t len = strlen(buffer_in);
            buffer_in[len] = '\n';
            buffer_in[len + 1] = '\0';
            Read_To_End_Of_Quote(file, &buffer_in[1], buffer_out, buffer_ex, buffer_out.Size());
        } else if (strcasecmp(buffer_in, "END") == 0) {
            ++count;
        }
    }

    count += 500; // #TODO Investigate

    file->Close();

    return true;
}

bool GameTextManager::Get_String_Count(const char *filename, int &count)
{
    return GameTextFile::Get_String_Count(filename,
        count,
        BufferView<char>::Create(m_bufferIn),
        BufferView<char>::Create(m_bufferOut),
        BufferView<char>::Create(m_bufferEx));
}

// Read info from a CSF file header.
bool GameTextFile::Get_CSF_Info(const char *filename, int &text_count, LanguageID &language)
{
    static_assert(sizeof(CSFHeader) == 24, "CSFHeader struct not expected size.");
    CSFHeader header;
    File *file = g_theFileSystem->Open(filename, File::BINARY | File::READ);

    if (file == nullptr || file->Read(&header, sizeof(header)) != sizeof(header)
        || header.id != FourCC<' ', 'F', 'S', 'C'>::value) {
        return false;
    }

    text_count = le32toh(header.num_labels);

    if (le32toh(header.version) <= 1) {
        m_language = LanguageID::US;
    } else {
        m_language = letoh<LanguageID>(header.langid);
    }

    file->Close();

    return true;
}

bool GameTextManager::Get_CSF_Info(const char *filename)
{
    return GameTextFile::Get_CSF_Info(filename, m_textCount, m_language);
}

// Parses older format string files which only support ascii.
bool GameTextFile::Parse_String_File(const char *filename,
    StringInfo *string_info,
    int &max_label_len,
    BufferView<unichar_t> buffer_trans,
    BufferView<char> buffer_in,
    BufferView<char> buffer_out,
    BufferView<char> buffer_ex)
{
    captainslog_info("Parsing string file '%s'.", filename);
    File *file = g_theFileSystem->Open(filename, File::TEXT | File::READ);

    if (file == nullptr) {
        return false;
    }

    int index = 0;
    bool end = false;

    while (Read_Line(buffer_in, buffer_in.Size(), file)) {
        Remove_Leading_And_Trailing(buffer_in);
        captainslog_trace("We have '%s' buffered.", m_bufferIn);

        // Skip line if it is empty or is a comment starting with "//".
        if (buffer_in[0] == '\0' || (buffer_in[0] == '/' && buffer_in[1] == '/')) {
            captainslog_trace("Line started with // or empty line. Skip.");
            continue;
        }

        end = false;

#if ASSERT_LEVEL >= ASSERTS_DEBUG
        if (index > 0) {
            for (int i = 0; i < index; ++i) {
                captainslog_dbgassert(strcasecmp(string_info[i].label.Str(), buffer_in) != 0,
                    "String label '%s' is defined multiple times!",
                    buffer_in);
            }
        }
#endif

        string_info[index].label = buffer_in;
        max_label_len = std::max<int>(strlen(buffer_in), max_label_len);

        bool read_string = false;

        while (Read_Line(buffer_in, buffer_in.Size() - 1, file)) {
            Remove_Leading_And_Trailing(buffer_in);
            captainslog_trace("We have '%s' buffered.", m_bufferIn);

            if (buffer_in[0] == '"') {
                size_t len = strlen(buffer_in);
                buffer_in[len] = '\n';
                buffer_in[len + 1] = '\0';
                Read_To_End_Of_Quote(file, buffer_in + 1, buffer_out, buffer_ex, buffer_out.Size());

                if (read_string) {
                    captainslog_trace("String label '%s' has more than one string defined!", buffer_in);

                    continue;
                }

                Translate_Copy(buffer_trans, buffer_out);
                Strip_Spaces(buffer_trans);

                // TODO maybe Windows build does something extra here not done in mac version.

                string_info[index].text = buffer_trans;
                string_info[index].speech = buffer_ex;

                read_string = true;
            } else if (strcasecmp(buffer_in, "END") == 0) {
                ++index;
                end = true;

                break;
            }
        }
    }

    file->Close();

    if (!end) {
        captainslog_error("Unexpected end of string file '%s'.", filename);

        return false;
    }

    return true;
}

bool GameTextManager::Parse_String_File(const char *filename)
{
    return GameTextFile::Parse_String_File(filename,
        m_stringInfo,
        m_maxLabelLen,
        BufferView<unichar_t>::Create(m_translateBuffer),
        BufferView<char>::Create(m_bufferIn),
        BufferView<char>::Create(m_bufferOut),
        BufferView<char>::Create(m_bufferEx));
}

// Parses CSF files which support UCS2 strings, essentially the BMP of unicode.
bool GameTextFile::Parse_CSF_File(const char *filename,
    StringInfo *string_info,
    int &max_label_len,
    BufferView<unichar_t> buffer_trans,
    BufferView<char> buffer_in)
{
    captainslog_info("Parsing CSF file '%s'.", filename);
    CSFHeader header;
    File *file = g_theFileSystem->Open(filename, File::BINARY | File::READ);

    if (file == nullptr || file->Read(&header, sizeof(header)) != sizeof(header)) {
        return false;
    }

    uint32_t id;
    int index = 0;

    if (file->Read(&id, sizeof(id)) != sizeof(id)) {
        file->Close();

        return false;
    }

    // Little endian "LBL " FourCC
    while (id == FourCC<' ', 'L', 'B', 'L'>::value) {
        int32_t num_strings;
        int32_t length;

        file->Read(&num_strings, sizeof(num_strings));
        file->Read(&length, sizeof(length));

        // convert to host endianess
        length = le32toh(length);
        num_strings = le32toh(num_strings);

        if (length > 0) {
            file->Read(buffer_in, length);
        }

        buffer_in[length] = '\0';
        string_info[index].label = buffer_in;
        max_label_len = std::max(length, max_label_len);

        // Read all strings associated with this label, Nox used multiple strings for
        // random variation, Generals only cares about first one.
        for (int i = 0; i < num_strings; ++i) {
            file->Read(&id, sizeof(id));

            if (id != FourCC<' ', 'R', 'T', 'S'>::value && id != FourCC<'W', 'R', 'T', 'S'>::value) {
                file->Close();

                return false;
            }

            file->Read(&length, sizeof(length));
            length = le32toh(length);

            if (length > 0) {
                file->Read(buffer_trans, sizeof(buffer_trans[0]) * length);
            }

            // CSF format supports multiple strings per label, but we only care about
            // first string.
            if (i == 0) {
                buffer_trans[length] = '\0';

                for (int j = 0; buffer_trans[j] != '\0'; ++j) {
                    // Correct for big endian systems
                    buffer_trans[j] = le16toh(buffer_trans[j]);

                    // Binary NOT to decode
                    buffer_trans[j] = ~buffer_trans[j];
                }

                Strip_Spaces(buffer_trans);
                string_info[index].text = buffer_trans;
            }

            // FourCC of 'STRW' rather than 'STR ' indicates extra data.
            if (id == FourCC<'W', 'R', 'T', 'S'>::value) {
                file->Read(&length, sizeof(length));
                length = le32toh(length);

                if (length > 0) {
                    file->Read(buffer_in, length);
                }

                buffer_in[length] = '\0';

                if (i == 0) {
                    string_info[index].speech = buffer_in;
                }
            }
        }

        ++index;

        if (file->Read(&id, sizeof(id)) != sizeof(id)) {
            break;
        }
    }

    file->Close();

    return true;
}

bool GameTextManager::Parse_CSF_File(const char *filename)
{
    return GameTextFile::Parse_CSF_File(filename,
        m_stringInfo,
        m_maxLabelLen,
        BufferView<unichar_t>::Create(m_translateBuffer),
        BufferView<char>::Create(m_bufferIn));
}

// Parse an additional string file for a map. Currently cannot be localized.
bool GameTextManager::Parse_Map_String_File(const char *filename)
{
    return GameTextFile::Parse_String_File(filename,
        m_mapStringInfo,
        m_maxLabelLen,
        BufferView<unichar_t>::Create(m_translateBuffer),
        BufferView<char>::Create(m_bufferIn),
        BufferView<char>::Create(m_bufferOut),
        BufferView<char>::Create(m_bufferEx));
}

GameTextManager::GameTextManager() :
    m_textCount(0),
    m_maxLabelLen(0),
    m_stringInfo(nullptr),
    m_stringLUT(nullptr),
    m_initialized(false),
    m_noStringList(nullptr),
    m_useStringFile(true),
    m_language(LanguageID::US),
    m_failed(U_CHAR("***FATAL*** String Manager failed to initialize properly")),
    m_mapStringInfo(nullptr),
    m_mapStringLUT(nullptr),
    m_mapTextCount(0),
    m_stringVector()
{
    memset(m_bufferIn, 0, sizeof(m_bufferIn));
    memset(m_bufferOut, 0, sizeof(m_bufferOut));
    memset(m_bufferEx, 0, sizeof(m_bufferEx));
}

GameTextManager::~GameTextManager()
{
    Reset(); // #BUGFIX Full cleanup on destruction.
    Deinit();

    // FEATURE: Perform full cleanup on destruction
    Reset();
}

void GameTextManager::Init()
{
    captainslog_info("Initializing GameTextManager.");
    if (m_initialized) {
        return;
    }

    Utf8String csfpath;
    bool use_csf = true;

    csfpath.Format("data/%s/Generals.csf", Get_Registry_Language().Str());

    // Check if we can use a standard string file, if not, try the csf file.
    if (m_useStringFile && Get_String_Count("data/Generals.str", m_textCount)) {
        use_csf = false;
    } else if (!Get_CSF_Info(csfpath.Str())) {
        captainslog_error("Couldn't get STR string count or CSF info, returning.");
        return;
    }

    if (m_textCount == 0) {
        captainslog_error("Text count is %d, returning.", m_textCount);
        return;
    }

    m_stringInfo = new StringInfo[m_textCount];

    if (m_stringInfo == nullptr) {
        captainslog_error("Couldn't allocated string info array.");
        Deinit();

        return;
    }

    // Try and parse the relevant string file, cleanup if not.
    if (!use_csf) {
        if (!Parse_String_File("data/Generals.str")) {
            captainslog_error("Couldn't parse string file.");

            Deinit();

            return;
        }
    } else {
        if (!Parse_CSF_File(csfpath.Str())) {
            captainslog_error("Couldn't parse compiled string file.");
            Deinit();

            return;
        }
    }

    // Generate the lookup table and sort it for efficient search.
    m_stringLUT = new StringLookUp[m_textCount];

    for (int i = 0; i < m_textCount; ++i) {
        m_stringLUT[i].info = &m_stringInfo[i];
        m_stringLUT[i].label = &m_stringInfo[i].label;
    }

    qsort(m_stringLUT, m_textCount, sizeof(StringLookUp), Compare_LUT);

    // Fetch the GUI window title string and set it here.
    Utf8String ntitle;
    Utf16String wtitle;
    wtitle = U_CHAR("Thyme - "); // #FEATURE Add Thyme text to window name.
    wtitle += Fetch("GUI:Command&ConquerGenerals");

    ntitle.Translate(wtitle);

#ifdef PLATFORM_WINDOWS
    if (g_applicationHWnd != 0) {
        SetWindowTextA(g_applicationHWnd, ntitle.Str());
        SetWindowTextW(g_applicationHWnd, (const wchar_t *)wtitle.Str());
    }
#endif
}

// Resets the map strings, main string file is left loaded.
void GameTextManager::Reset()
{
    m_mapTextCount = 0; // #BUGFIX Reset map text count as well.

    if (m_mapStringInfo != nullptr) {
        delete[] m_mapStringInfo;
        m_mapStringInfo = nullptr;
    }

    if (m_mapStringLUT != nullptr) {
        delete[] m_mapStringLUT;
        m_mapStringLUT = nullptr;
    }
}

// Find and return the unicode string corresponding to the label provided.
// Optionally can pass a bool pointer to determine if a string was found.
Utf16String GameTextManager::Fetch(Utf8String args, bool *success)
{
    return Fetch(args.Str(), success);
}

// Find and return the unicode string corresponding to the label provided.
// Optionally can pass a bool pointer to determine if a string was found.
Utf16String GameTextManager::Fetch(const char *args, bool *success)
{
    if (m_stringInfo == nullptr) {
        if (success != nullptr) {
            *success = false;
        }

        return m_failed;
    }

    Utf8String argstr = args;
    StringLookUp key = { &argstr, nullptr };

    StringLookUp *found =
        static_cast<StringLookUp *>(bsearch(&key, m_stringLUT, m_textCount, sizeof(StringLookUp), Compare_LUT));

    if (found != nullptr) {
        if (success != nullptr) {
            *success = true;
        }

        return found->info->text;
    }

    if (m_mapStringLUT != nullptr && m_mapTextCount > 0) {
        found =
            static_cast<StringLookUp *>(bsearch(&key, m_mapStringLUT, m_mapTextCount, sizeof(StringLookUp), Compare_LUT));

        if (found != nullptr) {
            if (success != nullptr) {
                *success = true;
            }

            return found->info->text;
        }
    }

    if (success != nullptr) {
        *success = false;
    }

    // If we reached here, we didn't find a string from our string file.
    Utf16String missing;
    NoString *no_string;

    missing.Format(U_CHAR("MISSING: '%hs'"), args);

    // Find missing string in NoString list if it already exists.
    for (no_string = m_noStringList; no_string != nullptr; no_string = no_string->next) {
        if (missing == no_string->text) {
            break;
        }
    }

    // If it was not found or the list was empty, add a new one.
    if (no_string == nullptr) {
        no_string = new NoString;
        no_string->text = missing;
        no_string->next = m_noStringList;
        m_noStringList = no_string;
    }

    return no_string->text;
}

// List all string labels that start with the prefix passed to the function.
std::vector<Utf8String> *GameTextManager::Get_Strings_With_Prefix(Utf8String label)
{
    m_stringVector.clear();

    captainslog_trace("Searching for strings prefixed with '%s'.", label.Str());

    // Search all string labels that start with the substring provided.
    if (m_stringLUT != nullptr) {
        for (int i = 0; i < m_textCount; ++i) {
            const char *lut_label = m_stringLUT[i].label->Str();

            if (strstr(lut_label, label.Str()) == lut_label) {
                m_stringVector.push_back(*m_stringLUT[i].label);
            }
        }
    }

    // Same again for map strings.
    if (m_mapStringLUT != nullptr) {
        for (int i = 0; i < m_mapTextCount; ++i) {
            const char *lut_label = m_mapStringLUT[i].label->Str();
            if (strstr(lut_label, label.Str()) == lut_label) {
                m_stringVector.push_back(*m_mapStringLUT[i].label);
            }
        }
    }

    return &m_stringVector;
}

// Initializes a string file associated with a specific map. Resources
// allocated here are freed by GameTextManager::Reset()
void GameTextManager::Init_Map_String_File(Utf8String const &filename)
{
    Get_String_Count(filename.Str(), m_mapTextCount);
    m_mapStringInfo = new StringInfo[m_mapTextCount];
    Parse_Map_String_File(filename.Str());

    // Generate the lookup table and sort it for efficient search.
    m_mapStringLUT = new StringLookUp[m_mapTextCount];

    for (int i = 0; i < m_mapTextCount; ++i) {
        m_mapStringLUT[i].info = &m_mapStringInfo[i];
        m_mapStringLUT[i].label = &m_mapStringInfo[i].label;
    }

    qsort(m_mapStringLUT, m_mapTextCount, sizeof(StringLookUp), Compare_LUT);
}

// Destroys the main string file, doesn't affect loaded map strings.
void GameTextManager::Deinit()
{
    if (m_stringInfo != nullptr) {
        delete[] m_stringInfo;
        m_stringInfo = nullptr;
    }

    if (m_stringLUT != nullptr) {
        delete[] m_stringLUT;
        m_stringLUT = nullptr;
    }

    m_textCount = 0;

    for (NoString *ns = m_noStringList; ns != nullptr;) {
        NoString *tmp = ns;
        ns = tmp->next;

        delete tmp;
    }

    m_noStringList = nullptr;
    m_initialized = false;
}

// #TODO Reorder class functions in this file to better match the order in header file.

GameTextFile::GameTextFile() : m_stringInfoCount(0), m_language(LanguageID::LANGUAGE_ID_US), m_stringInfo(nullptr) {}

GameTextFile::~GameTextFile()
{
    Unload();
}

bool GameTextFile::Load(const char *filename, GameTextType filetype)
{
    if (!filename || !filename[0]) {
        captainslog_error("String file without file name cannot be loaded");
        return false;
    }

    bool success = false;

    Unload();
    filetype = Get_File_Type(filename, filetype);

    int max_label_len = 0;
    char buffer_in[512] = { 0 };
    char buffer_out[512] = { 0 };
    char buffer_ex[512] = { 0 };
    unichar_t buffer_trans[1024] = { 0 };
    auto bufview_in = BufferView<char>::Create(buffer_in);
    auto bufview_out = BufferView<char>::Create(buffer_out);
    auto bufview_ex = BufferView<char>::Create(buffer_ex);
    auto bufview_trans = BufferView<unichar_t>::Create(buffer_trans);

    if (filetype == GameTextType::TYPE_CSF) {
        if (Get_CSF_Info(filename, m_stringInfoCount, m_language)) {
            captainslog_dbgassert(m_stringInfo == nullptr, "String info must be not allocated already");
            m_stringInfo = new StringInfo[m_stringInfoCount];
            if (Parse_CSF_File(filename, m_stringInfo, max_label_len, bufview_trans, bufview_in)) {
                success = true;
            }
        }
    } else if (filetype == GameTextType::TYPE_STR) {
        if (Get_String_Count(filename, m_stringInfoCount, bufview_in, bufview_out, bufview_ex)) {
            captainslog_dbgassert(m_stringInfo == nullptr, "String info must be not allocated already");
            m_stringInfo = new StringInfo[m_stringInfoCount];
            if (Parse_String_File(
                    filename, m_stringInfo, max_label_len, bufview_trans, bufview_in, bufview_out, bufview_ex)) {
                success = true;
            }
        }
    }

    static_assert(GameTextType::Count == static_cast<GameTextType>(3), "New Game Text Type needs to be covered here");

    if (success) {
        captainslog_info("String file '%s' with %d lines loaded successfully", filename, m_stringInfoCount);
    } else {
        Unload();
        captainslog_info("String file '%s' failed to load", filename);
    }

    return success;
}

void GameTextFile::Log_Length_Info(const GameTextLengthInfo &len_info)
{
    captainslog_info("String file max label len: %d", len_info.max_label_len);
    captainslog_info("String file max utf8 text len: %d", len_info.max_text_utf8_len);
    captainslog_info("String file max utf16 text len: %d", len_info.max_text_utf16_len);
    captainslog_info("String file max speech len: %d", len_info.max_speech_len);
}

void GameTextFile::Check_Length_Info(const GameTextLengthInfo &len_info)
{
    captainslog_dbgassert(GAMETEXT_BUFFER_SIZE > len_info.max_label_len, "Read buffer must be larger");
    captainslog_dbgassert(GAMETEXT_BUFFER_SIZE > len_info.max_text_utf8_len, "Read buffer must be larger");
    captainslog_dbgassert(GAMETEXT_TRANSLATE_SIZE > len_info.max_text_utf16_len, "Read buffer must be larger");
    captainslog_dbgassert(GAMETEXT_BUFFER_SIZE > len_info.max_speech_len, "Read buffer must be larger");
}

bool GameTextFile::Save(const char *filename, GameTextType filetype)
{
    if (!filename || !filename[0]) {
        captainslog_error("String file without file name cannot be saved");
        return false;
    }

    if (!m_stringInfo) {
        captainslog_error("String file without string data cannot be saved");
        return false;
    }

    File *file = g_theFileSystem->Open(filename, File::WRITE | File::CREATE | File::BINARY);
    if (!file) {
        captainslog_error("String file '%s' cannot be opened for save", filename);
        return false;
    }

    bool success = false;

    filetype = Get_File_Type(filename, filetype);

    GameTextLengthInfo len_info = { 0 };
    auto string_info_bufview = BufferView<StringInfo>::Create(m_stringInfo, m_stringInfoCount);

    if (filetype == GameTextType::TYPE_CSF) {
        if (Write_CSF_File(file, string_info_bufview, len_info, m_language)) {
            success = true;
        }
    } else if (filetype == GameTextType::TYPE_STR) {
        if (Write_STR_File(file, string_info_bufview, len_info)) {
            success = true;
        }
    }

    static_assert(GameTextType::Count == static_cast<GameTextType>(3), "New Game Text Type needs to be covered here");

    if (success) {
        Log_Length_Info(len_info);
        Check_Length_Info(len_info);
        captainslog_info("String file '%s' with %d text lines saved successfully", filename, m_stringInfoCount);
    } else {
        captainslog_info("String file '%s' failed to save", filename);
    }

    return success;
}

void GameTextFile::Unload()
{
    m_stringInfoCount = 0;
    m_language = LanguageID::LANGUAGE_ID_US;

    if (m_stringInfo) {
        delete[] m_stringInfo;
        m_stringInfo = nullptr;
    }
}

const char *GameTextFile::Get_File_Extension(const char *filename)
{
    const char *begin = filename;
    const char *end = filename + strlen(filename);
    while (end != begin) {
        if (*end == '.')
            return end + 1;
        --end;
    }
    return end;
}

GameTextType GameTextFile::Get_File_Type(const char *filename, GameTextType filetype)
{
    if (filetype == GameTextType::TYPE_AUTO) {
        const char *fileext = Get_File_Extension(filename);
        if (strcasecmp(fileext, "csf") == 0)
            filetype = GameTextType::TYPE_CSF;
        else if (strcasecmp(fileext, "str") == 0)
            filetype = GameTextType::TYPE_STR;
        else
            filetype = GameTextType::TYPE_CSF;
    }
    return filetype;
}

template<typename T> static bool GameTextFile::Write(File *file, const T &value)
{
    return file->Write(&value, sizeof(T)) == sizeof(T);
}

template<> bool GameTextFile::Write<Utf8String>(File *file, const Utf8String &string)
{
    const void *buf = string.Str();
    const int len = string.Get_Length() * sizeof(char);
    return file->Write(buf, len) == len;
}

template<> bool GameTextFile::Write<Utf16String>(File *file, const Utf16String &string)
{
    const void *buf = string.Str();
    const int len = string.Get_Length() * sizeof(unichar_t);
    return file->Write(buf, len) == len;
}

bool GameTextFile::Write(File *file, const void *data, size_t len)
{
    return file->Write(data, len) == len;
}

const char GameTextFile::s_eol[] = { '\r', '\n' };
const char GameTextFile::s_quo[] = { '"' };
const char GameTextFile::s_end[] = { 'E', 'N', 'D' };

bool GameTextFile::Write_STR_Label(File *file, const StringInfo &string_info, GameTextLengthInfo &len_info)
{
    bool ok = true;
    ok = ok && Write(file, string_info.label);
    ok = ok && Write(file, s_eol);
    if (ok) {
        len_info.max_label_len = std::max(len_info.max_label_len, string_info.label.Get_Length());
    }
    return ok;
}

bool GameTextFile::Write_STR_Text(
    File *file, const StringInfo &string_info, GameTextLengthInfo &len_info, GameTextOption options)
{
    Utf16String text_utf16;
    Utf8String text_utf8;

    // Copy utf16 text and treat certain characters special
    const bool write_lf = (options & GAMETEXTOPTION_WRITEOUT_LF);

    for (int i = 0, count = string_info.text.Get_Length(); i < count; ++i) {
        unichar_t c = string_info.text[i];
        if (write_lf && c == U_CHAR('\n')) {
            // Print an escaped line feed
            text_utf16.Concat(U_CHAR('\\'));
            text_utf16.Concat(U_CHAR('n'));
            if (i != 0) {
                // This is optional. Makes it easier to look at string in file.
                text_utf16.Concat(U_CHAR('\n'));
            }
        } else if (c == U_CHAR('"')) {
            // Print an escaped quote
            text_utf16.Concat(U_CHAR('\\'));
            text_utf16.Concat(U_CHAR('"'));
        } else {
            text_utf16.Concat(c);
        }
    }

    // Convert utf16 to utf8
    text_utf8.Translate(text_utf16);

    bool ok = true;
    ok = ok && Write(file, s_quo);
    ok = ok && Write(file, text_utf8);
    ok = ok && Write(file, s_quo);
    ok = ok && Write(file, s_eol);
    if (ok) {
        len_info.max_text_utf8_len = std::max(len_info.max_text_utf8_len, text_utf8.Get_Length());
        len_info.max_text_utf16_len = std::max(len_info.max_text_utf16_len, text_utf16.Get_Length());
    }
    return ok;
}

bool GameTextFile::Write_STR_Speech(File *file, const StringInfo &string_info, GameTextLengthInfo &len_info)
{
    bool ok = true;
    if (!string_info.speech.Is_Empty()) {
        ok = ok && Write(file, string_info.speech);
        ok = ok && Write(file, s_eol);
        if (ok) {
            len_info.max_speech_len = std::max(len_info.max_speech_len, string_info.speech.Get_Length());
        }
    }
    return ok;
}

bool GameTextFile::Write_STR_End(File *file, const StringInfo &string_info, GameTextLengthInfo &len_info)
{
    bool ok = true;
    ok = ok && Write(file, s_end);
    ok = ok && Write(file, s_eol);
    ok = ok && Write(file, s_eol);
    return ok;
}

bool GameTextFile::Write_STR_Entry(
    File *file, const StringInfo &string_info, GameTextLengthInfo &len_info, GameTextOption options)
{
    if (Write_STR_Label(file, string_info, len_info)) {
        if (Write_STR_Text(file, string_info, len_info, options)) {
            if (Write_STR_Speech(file, string_info, len_info)) {
                if (Write_STR_End(file, string_info, len_info)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool GameTextFile::Write_STR_File(File *file, BufferView<StringInfo> string_info_bufview, GameTextLengthInfo &len_info)
{
    captainslog_info("Writing string file '%s' in STR format", file->Get_File_Name().Str());

    for (size_t line = 0, count = string_info_bufview.Size(); line < count; ++line) {
        const StringInfo &string_info = string_info_bufview[line];
        if (!string_info.label.Is_Empty()) {
            if (!Write_STR_Entry(file, string_info_bufview[line], len_info)) {
                return false;
            }
        }
    }

    return true;
}

bool GameTextFile::Write_CSF_Header(File *file, BufferView<StringInfo> string_info_bufview, LanguageID language)
{
    CSFHeader header;
    header.id = FourCC_LE<'C', 'S', 'F', ' '>::value;
    header.version = 3;
    header.num_labels = string_info_bufview.Size();
    header.num_strings = string_info_bufview.Size();
    header.skip = FourCC_LE<'T', 'H', 'Y', 'M'>::value;
    header.langid = language;
    htole_ref(header.id);
    htole_ref(header.version);
    htole_ref(header.num_labels);
    htole_ref(header.num_strings);
    htole_ref(header.skip);
    htole_ref(header.langid);

    return Write(file, header);
}

bool GameTextFile::Write_CSF_Label(File *file, const StringInfo &string_info, GameTextLengthInfo &len_info)
{
    CSFLabelHeader header;
    header.id = FourCC_LE<'L', 'B', 'L', ' '>::value;
    header.string_count = 1;
    header.length = string_info.label.Get_Length();
    htole_ref(header.id);
    htole_ref(header.string_count);
    htole_ref(header.length);

    if (Write(file, header)) {
        if (Write(file, string_info.label)) {
            len_info.max_label_len = std::max(len_info.max_label_len, header.length);
            return true;
        }
    }

    return false;
}

bool GameTextFile::Write_CSF_Text(
    File *file, const StringInfo &string_info, GameTextLengthInfo &len_info, BufferView<unichar_t> translate_bufview)
{
    bool text_ok = false;
    bool speech_ok = false;
    bool write_speech = !string_info.speech.Is_Empty();

    {
        size_t text_len = string_info.text.Get_Length();

        CSFTextHeader header;
        header.id = write_speech ? FourCC_LE<'S', 'T', 'R', 'W'>::value : FourCC_LE<'S', 'T', 'R', ' '>::value;
        header.length = text_len;
        htole_ref(header.id);
        htole_ref(header.length);

        if (Write(file, header)) {

            captainslog_dbgassert(translate_bufview.Size() >= text_len, "Buffer is expected to be larger or equal text");
            text_len = std::min(text_len, translate_bufview.Size());

            for (size_t i = 0; i < text_len; ++i) {
                // Every char is binary flipped here by design.
                translate_bufview[i] = ~string_info.text[i];
                htole_ref(translate_bufview[i]);
            }

            if (Write(file, translate_bufview.Get(), text_len * sizeof(unichar_t))) {
                len_info.max_text_utf16_len = std::max(len_info.max_text_utf16_len, header.length);
                text_ok = true;
            }
        }
    }

    if (text_ok && write_speech) {
        CSFSpeechHeader header;
        header.length = string_info.speech.Get_Length();
        htole_ref(header.length);

        if (Write(file, header)) {
            if (Write(file, string_info.speech)) {
                len_info.max_speech_len = std::max(len_info.max_speech_len, header.length);
                speech_ok = true;
            }
        }
    }

    return text_ok && (speech_ok || !write_speech);
}

bool GameTextFile::Write_CSF_Entry(
    File *file, const StringInfo &string_info, GameTextLengthInfo &len_info, BufferView<unichar_t> translate_bufview)
{
    if (Write_CSF_Label(file, string_info, len_info)) {
        if (Write_CSF_Text(file, string_info, len_info, translate_bufview)) {
            return true;
        }
    }

    return false;
}

bool GameTextFile::Write_CSF_File(
    File *file, BufferView<StringInfo> string_info_bufview, GameTextLengthInfo &len_info, LanguageID language)
{
    captainslog_info("Writing string file '%s' in CSF format", file->Get_File_Name().Str());

    bool success = false;

    if (Write_CSF_Header(file, string_info_bufview, language)) {
        success = true;
        unichar_t translate[GAMETEXT_TRANSLATE_SIZE];
        auto translate_bufview = BufferView<unichar_t>::Create(translate);

        for (size_t line = 0, count = string_info_bufview.Size(); line < count; ++line) {
            const StringInfo &string_info = string_info_bufview[line];

            if (string_info.label.Is_Empty()) {
                captainslog_error("String %d has no label", line);
                continue;
            }

            if (!Write_CSF_Entry(file, string_info_bufview[line], len_info, translate_bufview)) {
                success = false;
                break;
            }
        }
    }

    return success;
}
