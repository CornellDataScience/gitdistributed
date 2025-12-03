#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include "message.hpp"

enum class TcpMode 
{
  SERVER,
  CLIENT
};

class TcpServer
{
private:
  void initialize_server();
  void initialize_client();

public:
  int port;
  TcpMode mode;
  int socket_fd;
  int connected_fd;

  TcpServer();
  TcpServer(int port, TcpMode mode);
  ~TcpServer();

  void init(int port, TcpMode mode);

  int connect(const std::string &server_address = "", const int port = 0);

  void send_message(Message& message, const int connected_fd);
  bool receive_message(char *buffer, const int connected_fd);
};