#pragma once
#include <string>
#include <unordered_map>

enum class MessageType : uint8_t
{
  PUSH = 0,
  PULL = 1,
  OK = 2
};

struct Message
{
  MessageType type;
  int commit_id;
  std::string data;
};

Message deserialize(char *buffer);
bool serialize(const Message &msg, char *buff);
Message handle_client_req(const Message &req);

class GitNode
{
private:
  // commit_id -> bytes (TODO)
  std::unordered_map<int, std::string> changes;
  bool isBackup;

  bool push(Message message);
  Message pull(Message message);
  int commit_id = 0;

public:
  GitNode();
  Message handle_client_req(const Message &req);
};
