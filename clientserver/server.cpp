#include "thread"
#include "git_app.hpp"
#include "tcp.hpp"
#include "commands.hpp"
#include "message.hpp"

#define BUFFER_SIZE 1024

TcpServer server;
GitApp gitApp;

void handle_connection(int connected_fd) {
    while (true) {
        std::vector<char> buffer = std::vector<char>(BUFFER_SIZE);
        
        bool received = server.receive_message(buffer.data(), connected_fd);
        std::cout << received << std::endl;
        if (received) {
            ClientRequest request = ClientRequest();
            std::cout << "Received client message" << std::endl;
            ClientRequest::deserialize(buffer.data(), request);
            std::cout << "Deserialized message" << std::endl;

            Command resp;
            // Critical section
            {
                std::unique_lock<std::mutex> gitapp_lock(gitApp.gitAppMutex);
                resp = gitApp.handle_client_req(Command{request.command_type, request.file_name, request.file_data.data()});
            }

            std::cout << "Handled request" << std::endl;
            ClientReply reply = ClientReply(resp);
            // std::cout << reply.command.file_name << std::endl;
            // std::cout << reply.command.data << std::endl;
            server.send_message(reply, connected_fd);
            std::cout << "Sent response to client " << connected_fd << std::endl;
        } else {
            // if errno == 0, connection is closed
            if (errno != 0) {
                std::cerr << "[ERROR] recv failed: " << strerror(errno) << std::endl;
            }
            
            return;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Please provide a port." << std::endl;
    }

    int port = std::stoi(argv[1]);

    std::cout << server.port << std::endl;
    std::cout << server.socket_fd << std::endl;
    std::cout << server.connected_fd << std::endl;

    server.init(port, TcpMode::SERVER);

    std::cout << server.port << std::endl;
    std::cout << server.socket_fd << std::endl;
    std::cout << server.connected_fd << std::endl;

    std::cout << "Server listening on port " << port << "\n";

    while (true) {
        int connected_fd = server.connect(); // inside connect(), create new thread for each connection

        // for each connection, run the listening/responding loop
        std::thread(handle_connection, connected_fd).detach();
    }
    
    return 0;
}