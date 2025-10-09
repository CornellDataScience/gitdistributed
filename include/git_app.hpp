#pragma once
#include <string>
#include <iostream>
#include <unordered_map>
#include "messages.hpp"

class GitApp
{
private:
  std::unordered_map<std::string, std::string> file_store;
  Message handle_push(const Message &req);
  Message handle_pull();

public:
  GitApp();
  Message handle_client_req(const Message &req);
};
