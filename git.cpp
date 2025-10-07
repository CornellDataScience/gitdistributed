#include "include/server.hpp"
#include "include/git.hpp"
#include "include/parse.hpp"

#define PORT 8080
#define BUFFER_SIZE 1024

GitNode::GitNode() {}

bool GitNode::push(Message message)
{
  changes[message.file_name] = std::string(message.data);
  return true;
}

Message GitNode::pull(Message message)
{
  Message msg;

  if (changes.empty())
  {
    msg.type = MessageType::SERVER_PULL;
    msg.data = "No commits available";
    return msg;
  }

  const auto& entry = *changes.begin();
  msg.type = MessageType::SERVER_PULL;
  msg.file_name = entry.first;
  msg.data.assign(entry.second.begin(), entry.second.end()); 

  return msg;
};

Message GitNode::handle_client_req(const Message &req) {
  Message msg;
  switch (req.type) {
    case MessageType::CLIENT_PUSH:
    {
      changes[msg.file_name] = req.data;

      msg.type = MessageType::SERVER_PUSH;
      msg.data = "Push successful";
      break;
    }
    case MessageType::CLIENT_PULL:
    {
      msg = pull(req);
      break;
    }
    default:
      break;
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
        bool received = server.receive_message(buffer);
        if (received) {
            Message req = deserialize(buffer);
            Message resp = gitNode.handle_client_req(req);
            server.send_message(resp);
        } else {
            std::cerr << "[ERROR] recv failed: " << strerror(errno) << std::endl;
        }
    }

    return 0;
}
