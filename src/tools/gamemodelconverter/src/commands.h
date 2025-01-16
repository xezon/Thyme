/**
 * @file
 *
 * @author xezon, DevGeniusCode
 *
 * @brief GameModel Converter Commands. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

// TODO gameGameModelfile.h
#include <gametextfile.h>
#include <memory>
#include <string>

namespace Thyme
{
using CommandId = size_t;
using GameModelOption = GameModelOption;
using GameModelOptions = GameModelFile::Options;
using GameModelFile = GameModelFile;
using GameModelFilePtr = std::shared_ptr<GameModelFile>;

enum class CommandActionId

{
    INVALID = -1,
    LOAD_W3D,
    LOAD_W3X,
    LOAD_BLEND,
    LOAD_MAX,
    SAVE_W3D,
    SAVE_W3X,
    SAVE_BLEND,
    SAVE_MAX,
    RESET,
    SET_OPTIONS,

    COUNT
};

enum class CommandArgumentId
{
    INVALID = -1,
    FILE_ID,
    FILE_PATH,
    OPTIONS,

    COUNT
};

enum class SimpleActionId
{
    OPTIONS,
    LOAD_W3D_FILE,
    LOAD_W3X_FILE,
    LOAD_BLEND_FILE,
    LOAD_MAX_FILE,
    SAVE_W3D,
    SAVE_W3X,
    SAVE_BLEND,
    SAVE_MAX,

    COUNT
};

constexpr size_t g_commandActionCount = size_t(CommandActionId::COUNT);
constexpr size_t g_commandArgumentCount = size_t(CommandArgumentId::COUNT);
constexpr size_t g_simpleActionCount = size_t(SimpleActionId::COUNT);

bool String_To_Command_Action_Id(const char *str, CommandActionId &action_id);
bool String_To_Command_Argument_Id(const char *str, CommandArgumentId &argument_id);
bool String_To_Simple_Action_Id(const char *str, SimpleActionId &action_id);

class Command
{
public:
    Command() : m_id(s_id++) {}
    virtual ~Command() {}

    CommandId Id() const { return m_id; }
    void Set_Id(CommandId id) { m_id = id; }

    virtual CommandActionId Type() const = 0;
    virtual bool Execute() const = 0;

private:
    CommandId m_id;
    static CommandId s_id;
};

class LoadW3DCommand : public Command
{
public:
    LoadW3DCommand(const GameModelFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}

    virtual CommandActionId Type() const override { return CommandActionId::LOAD_W3D; }
    virtual bool Execute() const override;

private:
    GameModelFilePtr m_filePtr;
    std::string m_filePath;
};

class LoadW3XCommand : public Command
{
public:
    LoadW3XCommand(const GameModelFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}

    virtual CommandActionId Type() const override { return CommandActionId::LOAD_W3X; }
    virtual bool Execute() const override;

private:
    GameModelFilePtr m_filePtr;
    std::string m_filePath;
};

class LoadBlendCommand : public Command
{
public:
    LoadBlendCommand(const GameModelFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}

    virtual CommandActionId Type() const override { return CommandActionId::LOAD_BLEND; }
    virtual bool Execute() const override;

private:
    GameModelFilePtr m_filePtr;
    std::string m_filePath;
};

class LoadMaxCommand : public Command
{
public:
    LoadMaxCommand(const GameModelFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}

    virtual CommandActionId Type() const override { return CommandActionId::LOAD_MAX; }
    virtual bool Execute() const override;

private:
    GameModelFilePtr m_filePtr;
    std::string m_filePath;
};

class SaveW3DCommand : public Command
{
public:
    SaveW3DCommand(const GameModelFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}

    virtual CommandActionId Type() const override { return CommandActionId::SAVE_W3D; }
    virtual bool Execute() const override;

private:
    GameModelFilePtr m_filePtr;
    std::string m_filePath;
};

class SaveW3XCommand : public Command
{
public:
    SaveW3XCommand(const GameModelFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}

    virtual CommandActionId Type() const override { return CommandActionId::SAVE_W3X; }
    virtual bool Execute() const override;

private:
    GameModelFilePtr m_filePtr;
    std::string m_filePath;
};

class SaveBlendCommand : public Command
{
public:
    SaveBlendCommand(const GameModelFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}

    virtual CommandActionId Type() const override { return CommandActionId::SAVE_BLEND; }
    virtual bool Execute() const override;

private:
    GameModelFilePtr m_filePtr;
    std::string m_filePath;
};

class SaveMaxCommand : public Command
{
public:
    SaveMaxCommand(const GameModelFilePtr &file_ptr, const char *path) : m_filePtr(file_ptr), m_filePath(path) {}

    virtual CommandActionId Type() const override { return CommandActionId::SAVE_MAX; }
    virtual bool Execute() const override;

private:
    GameModelFilePtr m_filePtr;
    std::string m_filePath;
};

class ResetCommand : public Command
{
public:
    ResetCommand(const GameModelFilePtr &file_ptr) : m_filePtr(file_ptr) {}

    virtual CommandActionId Type() const override { return CommandActionId::RESET; }
    virtual bool Execute() const override;

private:
    GameModelFilePtr m_filePtr;
};

class SetOptionsCommand : public Command
{
public:
    SetOptionsCommand(const GameModelFilePtr &file_ptr, GameModelOptions options) : m_filePtr(file_ptr), m_options(options) {}

    virtual CommandActionId Type() const override { return CommandActionId::SET_OPTIONS; }
    virtual bool Execute() const override;

private:
    GameModelFilePtr m_filePtr;
    GameModelOptions m_options;
};

} // namespace Thyme
