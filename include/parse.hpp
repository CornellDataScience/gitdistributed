#pragma once
#include <string>
#include <iostream>


enum class MessageType : uint8_t
{
  CLIENT_PUSH = 0,
  CLIENT_PULL = 1,
  SERVER_PUSH = 2,
  SERVER_PULL = 3
};

struct Message
{
  MessageType type;
  std::string file_name;
  std::string data;
};


Message deserialize(char *buffer);
bool serialize(const Message &msg, char *buff);
