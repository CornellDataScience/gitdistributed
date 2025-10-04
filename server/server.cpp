#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "server.hpp"
#include "git.hpp"

#define BUFFER_SIZE 1024

TcpServer::TcpServer(int port)
{
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    address.sin_family = AF_INET;         // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Any IP address
    address.sin_port = htons(port);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        std::cerr << "Socket creation failed\n";
    }

    // Forcefully attach socket to the port 8080
    if (setsockopt(server_fd, IPPROTO_TCP, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        std::cerr << "setsockopt failed\n";
        close(server_fd);
    }

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Bind failed\n";
        close(server_fd);
    }

    if (listen(server_fd, 3) < 0)
    {
        std::cerr << "Listen failed\n";
        close(server_fd);
    }
}

void TcpServer::connect()
{
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (new_socket < 0)
    {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        return;
    }
    std::cout << "Connection accepted\n";
}

bool TcpServer::receive(char *buffer)
{
    int bytes_received = recv(new_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        return true;
    }
    return false;
}

void TcpServer::respond(Message message)
{
    char resp_buff[BUFFER_SIZE] = {0};
    serialize(message, resp_buff);
    if (send(new_socket, resp_buff, strlen(resp_buff), 0) < 0)
    {
        std::cerr << "[ERROR] Failed to send message: " << strerror(errno) << std::endl;
    }
}