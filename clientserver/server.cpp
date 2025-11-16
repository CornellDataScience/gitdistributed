#include "git_app.hpp"
#include "tcp.hpp"
#include "commands.hpp"
#include "message.hpp"

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    GitApp gitApp;
    TcpServer server(PORT, TcpMode::SERVER);
   

    while (true) {
        std::cout << "Server listening on port " << PORT << "\n";

        std::vector<char> buffer = std::vector<char>(BUFFER_SIZE);

        server.connect();
        
        bool received = server.receive_message(buffer.data());
        std::cout << received << std::endl;
        if (received) {
            ClientRequest request = ClientRequest();
            std::cout << "Received client message" << std::endl;
            ClientRequest::deserialize(buffer.data(), request);
            std::cout << "Deserialized message" << std::endl;
            Command resp = gitApp.handle_client_req(Command{request.command_type, request.file_name, request.file_data.data()});
            std::cout << "Handled request" << std::endl;
            ClientReply reply = ClientReply(resp);
            // std::cout << reply.command.file_name << std::endl;
            // std::cout << reply.command.data << std::endl;
            server.send_message(reply);
            std::cout << "Sent response to client" << std::endl;
        } else {
            std::cerr << "[ERROR] recv failed: " << strerror(errno) << std::endl;
            return 1;
        }
    }

    return 0;
}