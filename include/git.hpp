#pragma once
#include <unordered_map>
#include "parse.hpp"

Message handle_client_req(const Message &req);

class GitNode
{
private:
  // file name -> file contents
  std::unordered_map<std::string, std::string> changes;

  bool push(Message message);
  Message pull(Message message);

public:
  GitNode();
  Message handle_client_req(const Message &req);
};
