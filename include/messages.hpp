#pragma once
#include <string>

enum class MessageType
{
  CLIENT_PUSH,
  CLIENT_PULL,
  SERVER_PUSH,
  SERVER_PULL
};

struct Message
{
  MessageType type;
  std::string file_name;
  std::string data;
};

Message deserialize(char *buffer);
size_t serialize(const Message &msg, char *buff);