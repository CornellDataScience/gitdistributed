#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include "messages.hpp"

enum class TcpMode 
{
  SERVER,
  CLIENT
};

class TcpServer
{
private:
  int port;
  TcpMode mode;
  int socket_fd;
  int connected_fd;

  void initialize_server();
  void initialize_client();

public:
  TcpServer(int port, TcpMode mode);
  ~TcpServer();

  void connect(const std::string &server_address = "");

  void send_message(Message message, const std::string &dest_address = "");
  bool receive_message(char *buffer, const std::string &source_address = "");
};
