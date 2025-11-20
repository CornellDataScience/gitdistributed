#include "thread"
#include "git_app.hpp"
#include "tcp.hpp"
#include "commands.hpp"
#include "message.hpp"

#define PORT 8080
#define BUFFER_SIZE 1024

TcpServer server(PORT, TcpMode::SERVER);
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
            std::cout << "Sent response to client" << std::endl;
        } else {
            std::cerr << "[ERROR] recv failed: " << strerror(errno) << std::endl;
        }
    }
}

int main() {
    std::cout << "Server listening on port " << PORT << "\n";

    while (true) {
        int connected_fd = server.connect(); // inside connect(), create new thread for each connection

        // for each connection, run the listening/responding loop
        std::thread t(handle_connection, connected_fd);
        t.detach();
    }
    
    return 0;
}