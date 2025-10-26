#include "commands.hpp"

int read_int(const char *buff)
{
  int value;
  memcpy(&value, buff, sizeof(int));
  buff += sizeof(int);
  return value;
}

char *read_bytes(const char *buff, int n)
{
  char *result = (char *)malloc(n);
  memcpy(result, buff, n);
  buff += n;
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
  case CommandType::SERVER_PULL:
  {
    int file_name_size = read_int(p);
    p += sizeof(int);
    char *file_name = read_bytes(p, file_name_size);
    p += file_name_size;
    int file_size = read_int(p);
    p += sizeof(int);
    char *file_contents = read_bytes(p, file_size);
    deserialized.file_name = std::string(file_name, file_name_size);
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

bool serializeCommand(const Command &cmd, char *buff)
{
  char *p = buff;
  writeInt(p, static_cast<int>(cmd.type));

  switch (cmd.type)
  {
  case CommandType::CLIENT_PUSH:
  case CommandType::SERVER_PULL:
    writeInt(p, static_cast<int>(cmd.file_name.size()));
    std::memcpy(p, cmd.file_name.data(), cmd.file_name.size());
    p += cmd.file_name.size();
    writeInt(p, static_cast<int>(cmd.data.size()));
    std::memcpy(p, cmd.data.data(), cmd.data.size());
    p += cmd.data.size();
    break;
  default:
    break;
  }
  return true;
}