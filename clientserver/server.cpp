#include "git_app.hpp"
#include "tcp.hpp"

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    GitApp gitApp;
    TcpServer server(PORT, TcpMode::SERVER);
    std::cout << "Server listening on port " << PORT << "\n";

    char buffer[BUFFER_SIZE] = {0};

    server.connect();

    while (true)
    {
        bool received = server.receive_message(buffer);
        if (received)
        {
            Message req = deserialize(buffer);
            Message resp = gitApp.handle_client_req(req);
            server.send_message(resp);
        }
        else
        {
            std::cerr << "[ERROR] recv failed: " << strerror(errno) << std::endl;
        }
    }

    return 0;
}