#include "parse.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_int(const char* buff) {
    int value;
    memcpy(&value, buff, sizeof(int));
    buff += sizeof(int);
    return value;
}

char* read_bytes(const char* buff, int n) {
    char *result = (char*) malloc(n);
    if (!result) {
        perror("MALLOC failure in read_bytes");
        exit(EXIT_FAILURE);
    }

    memcpy(result, buff, n);
    buff += n;

    return result;
}

Message deserialize(char *buffer) {
  Message deserialized;
  MessageType code = static_cast<MessageType>(read_int(buffer));

  deserialized.type = code;

  switch(code) {
    case MessageType::CLIENT_PUSH:
    case MessageType::SERVER_PULL:
        int file_name_size = read_int(buffer);
        char *file_name = read_bytes(buffer, file_name_size);
        int file_size = read_int(buffer);
        char *file_contents = read_bytes(buffer, file_size);
        deserialized.file_name = file_name;
        deserialized.data = file_contents;
        break;
  }
  return deserialized;
}


void writeInt(char* &buf, int value) {
    std::memcpy(buf, &value, sizeof(value));
    buf += sizeof(value);
}

bool serialize(const Message &msg, char *buff) {
  char* p = buff;
  writeInt(p, static_cast<int>(msg.type));

  switch (msg.type) {    
    case MessageType::CLIENT_PUSH:
    case MessageType::SERVER_PULL:
      writeInt(p, static_cast<int>(msg.file_name.size()));
      std::memcpy(p, msg.file_name.data(), msg.file_name.size());
      p += msg.file_name.size();
      writeInt(p, static_cast<int>(msg.data.size()));
      std::memcpy(p, msg.data.data(), msg.data.size());
      p += msg.data.size();
  }
  return true;
}