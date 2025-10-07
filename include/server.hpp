#pragma once
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <functional>
#include <netinet/in.h>
#include "parse.hpp"

class TcpServer
{
public:
  TcpServer(int port);
  void connect();
  bool receive_message(char *buffer);
  void send_message(Message message);
  // ~TcpServer();
  int server_fd;

private:
  std::string messageInput;
  struct sockaddr_in address;
  int new_socket;
  socklen_t addrlen;
};
