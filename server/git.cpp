#include <string>
#include <iostream>
#include "server.cpp"
#include "git.hpp"

#define PORT 8080
#define BUFFER_SIZE 1024

// TODO — ask about client implementation
Message deserialize(char *buffer)
{
  return Message();
}

bool serialize(const Message &msg, char *buff) {
  std::string serialized;
  switch (msg.type) {
    case MessageType::PULL:
    {
      serialized += msg.data;
      break;
    }
    case MessageType::PUSH:
    {
      serialized += msg.commit_id;
    }
    case MessageType::OK:
    {
      serialized += "OK";
    }
  }
  std::memcpy(buff, serialized.c_str(), serialized.size() + 1);
  return true;
}

bool GitNode::push(Message message)
{
  changes[message.commit_id] = std::string(message.data);
  return true;
}

Message GitNode::pull(Message message)
{
  Message msg;

  if (changes.empty())
  {
    std::cout << "No commits available";
    msg.type = MessageType::PULL;
    msg.commit_id = -1;
    msg.data = nullptr;
    return msg;
  }
  int mostRecentCommit = std::max_element(
        changes.begin(), 
        changes.end(),
        [](const auto &a, const auto &b) {
            return a.first < b.first;
        })->first;

  msg.type = MessageType::PULL;
  msg.commit_id = mostRecentCommit;

  msg.data = changes[mostRecentCommit].c_str();

  return msg;
};

Message GitNode::handle_client_req(const Message &req) {
  Message msg;
  switch (req.type) {
    case MessageType::PUSH:
    {
      commit_id++;
      changes[commit_id] = req.data;

      msg.type = MessageType::OK;
      msg.commit_id = commit_id;
      msg.data = "Push successful";
      break;
    }
    case MessageType::PULL:
    {
      msg = pull(req);
    }
    default:
    {
      msg.type = MessageType::OK;
      msg.data = "Unknown request";
      break;
    }
  }

  return msg;
}

int main() {
    GitNode gitNode;
    TcpServer server(PORT);
    std::cout << "Server listening on port " << PORT << "\n";

    char buffer[BUFFER_SIZE] = {0};

    server.connect();

    while (true) {
        bool received = server.receive(buffer);
        if (received) {
            Message req = deserialize(buffer);
            Message resp = gitNode.handle_client_req(req);
            server.respond(resp);
        } else {
            std::cerr << "[ERROR] recv failed: " << strerror(errno) << std::endl;
        }
    }

    return 0;
}
