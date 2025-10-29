#include "git_app.hpp"

GitApp::GitApp() {}

Message GitApp::handle_push(const Message &req)
{
  Message msg;
  file_store[req.file_name] = std::string(req.data);
  msg.type = MessageType::SERVER_PUSH;
  std::cout << "Handled push" << std::endl;
  return msg;
}

Message GitApp::handle_pull()
{
  Message msg;

  msg.type = MessageType::SERVER_PULL;

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

Message GitApp::handle_client_req(const Message &req)
{
  Message msg;
  switch (req.type)
  {
  case MessageType::CLIENT_PUSH:
  {
    msg = handle_push(req);
    break;
  }
  case MessageType::CLIENT_PULL:
  {
    msg = handle_pull();
    break;
  }
  default:
    break;
  }

  return msg;
}
