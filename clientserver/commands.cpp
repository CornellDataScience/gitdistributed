#include "commands.hpp"
#include <iostream>

int read_int(const char *buff)
{
  int value;
  memcpy(&value, buff, sizeof(int));
  // buff += sizeof(int);
  return value;
}

char *read_bytes(const char *buff, int n)
{
  char *result = (char *)malloc(n);
  memcpy(result, buff, n);
  std::cout << "n: " << n << std::endl;
  for (int i = 0; i < n; i++) {
    std::cout << buff[i];
  }
  std::cout << std::endl;
  // buff += n;
  return result;
}

Command deserializeCommand(char *buffer)
{
  Command deserialized;
  char *p = buffer;

  CommandType code = static_cast<CommandType>(read_int(p));
  std::cout << "read command type " << read_int(p) << std::endl;
  p += sizeof(int);
  deserialized.type = code;


  switch (code)
  {
  case CommandType::CLIENT_PUSH:
  case CommandType::SERVER_PULL:
  {
    int file_name_size = read_int(p);
    std::cout << "read file name size " << file_name_size << std::endl;
    p += sizeof(int);
    char *file_name = read_bytes(p, file_name_size);
    std::cout << "read file name ";
    p += file_name_size;
    deserialized.file_name = std::string(file_name, file_name_size);
    std::cout << deserialized.file_name << std::endl;

    int file_size = read_int(p);
    std::cout << "read file size " << file_size << std::endl;
    p += sizeof(int);
    char *file_contents = read_bytes(p, file_size);
    std::cout << "read file contents";
    deserialized.data = std::string(file_contents, file_size);
    std::cout << deserialized.data << std::endl;

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
  // std::cout << "Size of buffer: " + static_cast<size_t>(p - buff) << std::endl;
  return static_cast<size_t>(p - buff);
}