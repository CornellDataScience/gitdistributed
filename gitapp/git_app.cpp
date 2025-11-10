#include "git_app.hpp"
#include "commands.hpp"
#include "message.hpp"

GitApp::GitApp() {}

Command GitApp::handle_push(const Command &req)
{
  Command msg;
  file_store[req.file_name] = std::string(req.data);
  msg.type = CommandType::SERVER_PUSH;
  std::cout << "Handled push" << std::endl;
  return msg;
}

Command GitApp::handle_pull()
{
  Command msg;

  msg.type = CommandType::SERVER_PULL;

  if (file_store.empty())
  {
    msg.data = "No commits available";
    return msg;
  }

  const auto &entry = *file_store.begin();
  msg.file_name = entry.first;
  msg.data.assign(entry.second.begin(), entry.second.end());

  return msg;
};

Command GitApp::handle_client_req(const Command &req)
{
  Command msg;
  switch (req.type)
  {
  case CommandType::CLIENT_PUSH:
  {
    msg = handle_push(req);
    break;
  }
  case CommandType::CLIENT_PULL:
  {
    msg = handle_pull();
    break;
  }
  default:
    break;
  }

  return msg;
}
