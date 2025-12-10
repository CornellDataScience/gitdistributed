#include "thread"
#include "git_app.hpp"
#include "tcp.hpp"
#include "commands.hpp"
#include "message.hpp"
#include "viewserver.hpp"

#define SERVER_IP "127.0.0.1"

#define VIEWSERVER_IP "127.0.0.1"
#define VIEWSERVER_PORT 8067
#define PING_INTERVAL_MS 500

#define BUFFER_SIZE 1024

TcpServer server;
GitApp gitApp;

std::string server_id;

View view;
int backup_fd = -1;

void forward_request(ClientRequest request, int backup_fd);

void handle_connection(int connected_fd) {
    while (true) {
        bool is_primary = server_id == view.primary;
        std::cout << "is_primary: " << is_primary << std::endl;
        
        std::vector<char> buffer = std::vector<char>(BUFFER_SIZE);
        
        bool received = server.receive_message(buffer.data(), connected_fd);
        std::cout << received << std::endl;
        if (received) {
            if (is_primary) {
                ClientRequest request = ClientRequest();
                std::cout << "Received client message" << std::endl;
                ClientRequest::deserialize(buffer.data(), request);
                std::cout << "Deserialized message" << std::endl;

                // Forward request
                if (view.backup != "") {
                    forward_request(request, backup_fd);
                }

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
            }
            else {
                //backup
                ForwardedRequest request = ForwardedRequest();
                std::cout << "Received forwarded request" << std::endl;
                ForwardedRequest::deserialize(buffer.data(), request);
                std::cout << "Deserialized message" << std::endl;

                Command resp;
                // Critical section
                {
                    std::unique_lock<std::mutex> gitapp_lock(gitApp.gitAppMutex);
                    resp = gitApp.handle_client_req(Command{request.client_request.command_type, request.client_request.file_name, request.client_request.file_data.data()});
                }

                std::cout << "Handled request" << std::endl;
                BackupReply reply = BackupReply(request, server_id);
                // std::cout << reply.command.file_name << std::endl;
                // std::cout << reply.command.data << std::endl;
                server.send_message(reply, connected_fd);
                std::cout << "Sent response to primary " << connected_fd << std::endl;
            }
        } else {
            // if errno == 0, connection is closed
            if (errno != 0) {
                std::cerr << "[ERROR] recv failed: " << strerror(errno) << std::endl;
            }
            
            return;
        }
    }
}

void forward_request(ClientRequest request, int backup_fd) {
    std::cout << "backup_fd: " << backup_fd << std::endl;
    ForwardedRequest forwardRequest = ForwardedRequest(request, server_id);
    server.send_message(forwardRequest, backup_fd);
    std::cout << "Sent forwarded request" << std::endl;

    bool received = server.receive_message(std::vector<char>(BUFFER_SIZE).data(), backup_fd);
    if (received) {
        std::cout << "Received backup reply" << std::endl;
    }
}

void ping(int viewserver_fd) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(PING_INTERVAL_MS));
        // want: message containing view, server_id
        Ping myPing = Ping{view.view_num, server_id};
        server.send_message(myPing, viewserver_fd);

        std::vector<char> buffer = std::vector<char>(BUFFER_SIZE);
        bool received = server.receive_message(buffer.data(), viewserver_fd);
        if (received)
        {
            ViewReply req;
            ViewReply::deserialize(buffer.data(), req);
            
            view = View{req.view_num, req.primary, req.backup};
            std::cout << "view: " << view.view_num << " " << view.primary << " " << view.backup << std::endl;
        }
        else
        {
            std::cerr << "[ERROR] request failed: " << strerror(errno) << std::endl;
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

    server_id = SERVER_IP + std::string(":") + argv[1];

    // Initialize port to listen
    server.init(port, TcpMode::SERVER);

    // Create connection to view server and start pinging
    int viewserver_fd = server.connect(VIEWSERVER_IP, VIEWSERVER_PORT);
    std::thread(ping, viewserver_fd).detach();

    while (view.view_num != 2) {
        // busy wait
    }
    
    bool is_primary = server_id == view.primary;

    if (is_primary) {
        // Connect to backup
        int colon_index = view.backup.find(":");
        std::string backup_ip = view.backup.substr(0, colon_index);
        int backup_port = std::stoi(view.backup.substr(colon_index + 1));

        backup_fd = server.connect(backup_ip, backup_port);
        std::thread(handle_connection, backup_fd).detach();

        std::cout << "Connected to backup " << view.backup << std::endl;
    }

    std::cout << server.port << std::endl;
    std::cout << server.socket_fd << std::endl;
    std::cout << server.connected_fd << std::endl;

    std::cout << "Server listening on port " << port << "\n";

    while (true) {
        int connected_fd = server.connect(); // inside connect(), create new thread for each connection
        std::cout << "server fd" << std::endl;
        std::cout << connected_fd << std::endl;
        // for each connection, run the listening/responding loop
        std::thread(handle_connection, connected_fd).detach();
    }
    
    return 0;
}