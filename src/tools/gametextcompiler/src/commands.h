/**
 * @file
 *
 * @author xezon
 *
 * @brief Game Text Compiler Commands
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include <gametextfile.h>
#include <memory>
#include <string>

namespace Thyme
{
using CommandId = int;
using GameTextOption = GameTextOption;
using GameTextOptions = GameTextFile::Options;
using GameTextFile = GameTextFile;
using GameTextFilePtr = std::shared_ptr<GameTextFile>;
using Languages = GameTextFile::Languages;

enum class CommandActionId
{
    INVALID = -1,
    LOAD,
    LOAD_CSF,
    LOAD_STR,
    SAVE,
    SAVE_CSF,
    SAVE_STR,
    UNLOAD,
    RESET,
    MERGE_AND_OVERWRITE,
    SET_OPTIONS,
    SET_LANGUAGE,
    SWAP_LANGUAGE_STRINGS,
};

enum class CommandArgumentId
{
    INVALID = -1,
    FILE_ID,
    FILE_PATH,
    LANGUAGES,
    OPTIONS,
};

bool String_To_Command_Action(const char *str, CommandActionId &action_id);
bool String_To_Command_Argument(const char *str, CommandArgumentId &argument_id);

class Command
{
public:
    Command() : m_id(s_id++) {}
    virtual ~Command() {}
    CommandId Id() const { return m_id; }

    virtual CommandActionId Type() const = 0;
    virtual bool Execute() = 0;

private:
    CommandId m_id;
    static CommandId s_id;
};

class LoadCommand : public Command
{
public:
    LoadCommand(const GameTextFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}

    virtual CommandActionId Type() const override { return CommandActionId::LOAD; }
    virtual bool Execute() override { return m_filePtr->Load(m_filePath.c_str()); }

private:
    GameTextFilePtr m_filePtr;
    std::string m_filePath;
};

class SaveCommand : public Command
{
public:
    SaveCommand(const GameTextFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}

    virtual CommandActionId Type() const override { return CommandActionId::SAVE; }
    virtual bool Execute() override { return m_filePtr->Save(m_filePath.c_str()); }

private:
    GameTextFilePtr m_filePtr;
    std::string m_filePath;
};

class LoadCsfCommand : public Command
{
public:
    LoadCsfCommand(const GameTextFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}

    virtual CommandActionId Type() const override { return CommandActionId::LOAD_CSF; }
    virtual bool Execute() override { return m_filePtr->Load_CSF(m_filePath.c_str()); }

private:
    GameTextFilePtr m_filePtr;
    std::string m_filePath;
};

class SaveCsfCommand : public Command
{
public:
    SaveCsfCommand(const GameTextFilePtr &file_ptr, const char *path, LanguageID language) :
        m_filePtr(file_ptr), m_filePath(path), m_language(language)
    {
    }

    virtual CommandActionId Type() const override { return CommandActionId::SAVE_CSF; }
    virtual bool Execute() override
    {
        LanguageID old_language = m_filePtr->Get_Language();
        if (m_language != LanguageID::UNKNOWN) {
            m_filePtr->Set_Language(m_language);
        }
        bool success = m_filePtr->Save_CSF(m_filePath.c_str());
        m_filePtr->Set_Language(old_language);
        return success;
    }

private:
    GameTextFilePtr m_filePtr;
    std::string m_filePath;
    LanguageID m_language;
};

class LoadStrCommand : public Command
{
public:
    LoadStrCommand(const GameTextFilePtr &file_ptr, const char *path, Languages languages) :
        m_filePtr(file_ptr), m_filePath(path), m_languages(languages)
    {
    }
    virtual CommandActionId Type() const override { return CommandActionId::LOAD_STR; }
    virtual bool Execute() override
    {
        if (m_languages.any()) {
            return m_filePtr->Load_STR(m_filePath.c_str(), m_languages);
        } else {
            return m_filePtr->Load_STR(m_filePath.c_str());
        }
    }

private:
    GameTextFilePtr m_filePtr;
    std::string m_filePath;
    Languages m_languages;
};

class SaveStrCommand : public Command
{
public:
    SaveStrCommand(const GameTextFilePtr &file_ptr, const char *path, Languages languages) :
        m_filePtr(file_ptr), m_filePath(path), m_languages(languages)
    {
    }
    virtual CommandActionId Type() const override { return CommandActionId::SAVE_STR; }
    virtual bool Execute() override
    {
        if (m_languages.any()) {
            return m_filePtr->Save_STR(m_filePath.c_str(), m_languages);
        } else {
            return m_filePtr->Save_STR(m_filePath.c_str());
        }
    }

private:
    GameTextFilePtr m_filePtr;
    std::string m_filePath;
    Languages m_languages;
};

class UnloadCommand : public Command
{
public:
    UnloadCommand(const GameTextFilePtr &file_ptr, Languages languages) : m_filePtr(file_ptr), m_languages(languages) {}

    virtual CommandActionId Type() const override { return CommandActionId::UNLOAD; }
    virtual bool Execute() override
    {
        if (m_languages.any()) {
            m_filePtr->Unload(m_languages);
        } else {
            m_filePtr->Unload();
        }
        return true;
    }

private:
    GameTextFilePtr m_filePtr;
    Languages m_languages;
};

class ResetCommand : public Command
{
public:
    ResetCommand(const GameTextFilePtr &file_ptr) : m_filePtr(file_ptr) {}

    virtual CommandActionId Type() const override { return CommandActionId::RESET; }
    virtual bool Execute() override
    {
        m_filePtr->Reset();
        return true;
    }

private:
    GameTextFilePtr m_filePtr;
};

class MergeAndOverwriteCommand : public Command
{
public:
    MergeAndOverwriteCommand(const GameTextFilePtr &file_ptr_a, const GameTextFilePtr &file_ptr_b, Languages languages) :
        m_filePtrA(file_ptr_a), m_filePtrB(file_ptr_b), m_languages(languages)
    {
    }
    virtual CommandActionId Type() const override { return CommandActionId::MERGE_AND_OVERWRITE; }
    virtual bool Execute() override
    {
        if (m_languages.any()) {
            m_filePtrA->Merge_And_Overwrite(*m_filePtrB, m_languages);
        } else {
            m_filePtrA->Merge_And_Overwrite(*m_filePtrB);
        }
        return true;
    }

private:
    GameTextFilePtr m_filePtrA;
    GameTextFilePtr m_filePtrB;
    Languages m_languages;
};

class SetOptionsCommand : public Command
{
public:
    SetOptionsCommand(const GameTextFilePtr &file_ptr, GameTextOptions options) : m_filePtr(file_ptr), m_options(options) {}

    virtual CommandActionId Type() const override { return CommandActionId::SET_OPTIONS; }
    virtual bool Execute() override
    {
        m_filePtr->Set_Options(m_options);
        return true;
    }

private:
    GameTextFilePtr m_filePtr;
    GameTextOptions m_options;
};

class SetLanguageCommand : public Command
{
public:
    SetLanguageCommand(const GameTextFilePtr &file_ptr, LanguageID language) : m_filePtr(file_ptr), m_language(language) {}

    virtual CommandActionId Type() const override { return CommandActionId::SET_LANGUAGE; }
    virtual bool Execute() override
    {
        m_filePtr->Set_Language(m_language);
        return true;
    }

private:
    GameTextFilePtr m_filePtr;
    LanguageID m_language;
};

class SwapLanguageStringsCommand : public Command
{
public:
    SwapLanguageStringsCommand(const GameTextFilePtr &file_ptr, LanguageID language_a, LanguageID language_b) :
        m_filePtr(file_ptr), m_languageA(language_a), m_languageB(language_b)
    {
    }
    virtual CommandActionId Type() const override { return CommandActionId::SWAP_LANGUAGE_STRINGS; }
    virtual bool Execute() override
    {
        m_filePtr->Swap_String_Infos(m_languageA, m_languageB);
        return true;
    }

private:
    GameTextFilePtr m_filePtr;
    LanguageID m_languageA;
    LanguageID m_languageB;
};

} // namespace Thyme
