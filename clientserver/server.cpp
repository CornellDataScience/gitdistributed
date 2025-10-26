#include "git_app.hpp"
#include "tcp.hpp"

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    GitApp gitApp;
    TcpServer server(PORT, TcpMode::SERVER);
    std::cout << "Server listening on port " << PORT << "\n";

    char buffer[BUFFER_SIZE];

    server.connect();

    while (true)
    {
        std::cout << "Before receive" << std::endl;
        
        bool received = server.receive_message(buffer);
        std::cout << received << std::endl;
        if (received)
        {
            std::cout << "Received client message" << std::endl;
            Message req = deserialize(buffer);
            std::cout << "Deserialized message" << std::endl;
            Message resp = gitApp.handle_client_req(req);
            std::cout << "Handled request" << std::endl;
            server.send_message(resp);
            std::cout << "Sent response to client" << std::endl;
        }
        else
        {
            std::cerr << "[ERROR] recv failed: " << strerror(errno) << std::endl;
            return 1;
        }
    }

    return 0;
}