#include "tcp.hpp"
#include "commands.hpp"
#include "message.hpp"

#define BUFFER_SIZE 1024

TcpServer::TcpServer(int port, TcpMode mode) : port(port), mode(mode), socket_fd(-1), connected_fd(-1)
{
    if (mode == TcpMode::SERVER)
    {
        initialize_server();
    }
    else
    {
        initialize_client();
    }
}

void TcpServer::initialize_server()
{
    struct sockaddr_in address;
    int opt = 1;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        std::cerr << "Socket creation failed\n";
        return;
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        std::cerr << "setsockopt failed\n";
        close(socket_fd);
        return;
    }

    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Bind failed\n";
        close(socket_fd);
        return;
    }

    if (listen(socket_fd, 3) < 0)
    {
        std::cerr << "Listen failed\n";
        close(socket_fd);
        return;
    }
}

void TcpServer::initialize_client()
{
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        std::cerr << "Socket creation failed\n";
        return;
    }
}

void TcpServer::connect(const std::string &server_address)
{
    if (mode == TcpMode::SERVER)
    {
        // Server: accept incoming connection
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        connected_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connected_fd < 0)
        {
            std::cerr << "Accept failed: " << strerror(errno) << std::endl;
            return;
        }
        std::cout << "Connection accepted with fd " << connected_fd << std::endl;
    }
    else
    {
        // Client: connect to server
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, server_address.c_str(), &serv_addr.sin_addr) <= 0)
        {
            std::cerr << "Invalid address: " << server_address << std::endl;
            return;
        }

        if (::connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            std::cerr << "Connection failed: " << strerror(errno) << std::endl;
            return;
        }
        
        connected_fd = socket_fd;
        std::cout << "Connected to server with fd " << connected_fd << std::endl;
    }
}

static bool send_all(int fd, const char* data, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = send(fd, data + total, len - total, 0);
        if (n <= 0) return false;
        total += static_cast<size_t>(n);
    }
    return true;
}

void TcpServer::send_message(Message &message, const std::string &dest_address)
{
    std::vector<char> resp_buff = Message::serialize(message);

    int nbytes = resp_buff.size();

    if (!send_all(connected_fd, resp_buff.data(), nbytes)) {
        std::cerr << "[ERROR] Failed to send message: " << strerror(errno) << std::endl;
    }

    std::cout << "Sending " << nbytes << " bytes to fd " << connected_fd << std::endl;
}

bool TcpServer::receive_message(char *buffer, const std::string &source_address)
{
    int target_fd = connected_fd;
    std::cout << "Waiting for receive from fd " << target_fd << std::endl;
    std::cout << "Socket_fd " << socket_fd << std::endl;
    
    int bytes_received = recv(target_fd, buffer, BUFFER_SIZE - 1, 0);
    std::cout << bytes_received << std::endl;
    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        return true;
    } else {
        std::cout << "errno = " << strerror(errno) << std::endl;
        
        return false;
    }
}

TcpServer::~TcpServer()
{
    if (connected_fd >= 0 && connected_fd != socket_fd)
    {
        close(connected_fd);
    }
    if (socket_fd >= 0)
    {
        close(socket_fd);
    }
}