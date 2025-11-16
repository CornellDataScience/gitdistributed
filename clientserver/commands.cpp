#include "commands.hpp"
#include <iostream>

int read_int(const char *buff)
{
  int value;
  memcpy(&value, buff, sizeof(int));
  return value;
}

char *read_bytes(const char *buff, int n)
{
  char *result = (char *)malloc(n);
  memcpy(result, buff, n);
  return result;
}

Command deserializeCommand(char *buffer)
{
  Command deserialized;
  char *p = buffer;

  CommandType code = static_cast<CommandType>(read_int(p));
  p += sizeof(int);
  deserialized.type = code;

  switch (code)
  {
  case CommandType::CLIENT_PUSH:
  case CommandType::SERVER_PUSH:
  case CommandType::SERVER_PULL:
  {
    int file_name_size = read_int(p);
    p += sizeof(int);

    char *file_name = read_bytes(p, file_name_size);
    p += file_name_size;
    deserialized.file_name = std::string(file_name, file_name_size);

    int file_size = read_int(p);
    p += sizeof(int);
    char *file_contents = read_bytes(p, file_size);
    deserialized.data = std::string(file_contents, file_size);

    free(file_name);
    free(file_contents);
    break;
  }
  default:
    break;
  }
  return deserialized;
}

void writeInt(char *&buf, int value)
{
  std::memcpy(buf, &value, sizeof(value));
  buf += sizeof(value);
}

size_t serializeCommand(const Command &msg, char *buff)
{
  char *p = buff;

  auto writeInt = [&](int value) {
    std::memcpy(p, &value, sizeof(int));
    p += sizeof(int);
  };

  writeInt(static_cast<int>(msg.type));

  switch (msg.type)
  {
    case CommandType::CLIENT_PUSH:
    case CommandType::SERVER_PUSH:
    case CommandType::SERVER_PULL:
    {
      int nameSize = static_cast<int>(msg.file_name.size());
      writeInt(nameSize);
      std::memcpy(p, msg.file_name.data(), nameSize);
      p += nameSize;

      int dataSize = static_cast<int>(msg.data.size());
      writeInt(dataSize);
      std::memcpy(p, msg.data.data(), dataSize);
      p += dataSize;
      break;
    }
    default:
      break;
  }
  // std::cout << "Size of buffer: ";
  // std::cout << static_cast<size_t>(p - buff) << std::endl;
  return static_cast<size_t>(p - buff);
}