#pragma once
#include <string>

enum class CommandType
{
  CLIENT_PUSH,
  CLIENT_PULL,
  SERVER_PUSH,
  SERVER_PULL
};

struct Command
{
  CommandType type;
  std::string file_name;
  std::string data;
};

Command deserializeCommand(char *buffer);
size_t serializeCommand(const Command &cmd, char *buff);