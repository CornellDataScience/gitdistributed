#include <string>
#include <iostream>
#include <functional>
#include <netinet/in.h>

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
