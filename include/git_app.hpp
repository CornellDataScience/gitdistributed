#pragma once
#include <string>
#include <iostream>
#include <unordered_map>
#include "commands.hpp"

class GitApp
{
private:
  std::unordered_map<std::string, std::string> file_store;
  Command handle_push(const Command &req);
  Command handle_pull();

public:
  std::mutex gitAppMutex;
  
  GitApp();
  Command handle_client_req(const Command &req);
};
