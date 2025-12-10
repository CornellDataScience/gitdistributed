#include <iostream>
#include <filesystem>
#include <fstream>
#include "tcp.hpp"
#include "commands.hpp"
#include "message.hpp"
#include <viewserver.hpp>

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

using namespace std;
using namespace std::filesystem;

#define VIEWSERVER_IP "127.0.0.1"
#define VIEWSERVER_PORT 8067
#define PING_INTERVAL_MS 500

View view;

std::string server_id = "client";

TcpServer server(PORT, TcpMode::CLIENT);

void ping(int viewserver_fd);

void init() {
    path folder = ".gitd";
    if (!exists(folder)) { create_directory(folder); }
    
    path subfolder1 = ".gitd/objects/", subfolder2 = ".gitd/commits/";
    if (!exists(subfolder1)) { create_directory(subfolder1); }
    if (!exists(subfolder2)) { create_directory(subfolder2); }
}

void add(string name) {
    copy(name, ".gitd/objects/" + name);
}

void commit() {
    const path gitdpath = ".gitd";

    if (!exists(gitdpath)) {
        cout << "not a gitd repository" << endl;
        return;
    }

    const path objectspath = ".gitd/objects/";
    const path commitspath = ".gitd/commits/";

    if (!exists(objectspath) || !exists(commitspath)) {
        cout << "invalid gitd repository" << endl;
    }

    // Assuming multiple files and no overriding commits
    for (const auto& file : directory_iterator(objectspath)) {
        try {
            rename(file.path(), commitspath / file.path().filename());
        } catch (const filesystem_error& e) {
            cout << "error moving file " << file.path() << ": " << e.what() << endl;
        }
    }

    return;
}

void push() {
    int viewserver_fd = server.connect(VIEWSERVER_IP, VIEWSERVER_PORT);
    std::thread(ping, viewserver_fd).detach();

    while (view.view_num == 0) {
        // busy wait
    }

    int colon_index = view.primary.find(":");
    std::string primary_ip = view.primary.substr(0, colon_index);
    int primary_port = std::stoi(view.primary.substr(colon_index + 1));

    int connected_fd = server.connect(primary_ip, primary_port);
    std::cout << "Client connected to " << primary_ip << ":" << primary_port << "\n";

    ClientRequest request;
    
    const path commitspath = ".gitd/commits/";
    
    // Assuming only one file in commitsfolder
    for (const auto& file : directory_iterator(commitspath)) {
        try {
            std::string file_name = file.path().filename().string();
            std::cout << "Found file: " << file_name << std::endl;

            // Open the file
            std::ifstream input_file(file.path());

            // Check if the file opened successfully
            if (input_file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(input_file)),
                                    std::istreambuf_iterator<char>());
                
                // Initialize client request fields
                request = ClientRequest(CommandType::CLIENT_PUSH, file_name, content);
            } else {
                std::cerr << "Failed to open file: " << file.path() << std::endl;
            }
        } catch (const filesystem_error& e) {
            cout << "error reading file " << file.path() << ": " << e.what() << endl;
        }
    }

    // TODO: handle deleting from commits folder

    // sleep(5);
    std::cout << "Sending message" << std::endl;

    // make continuous requests
    server.send_message(request, connected_fd);

    std::vector<char> buffer = std::vector<char>(BUFFER_SIZE);
    bool received = server.receive_message(buffer.data(), connected_fd);
    if (received)
    {
        ClientReply req;

        ClientReply::deserialize(buffer.data(), req);
        
        // std::cout << req.command.data << std::endl;
        // std::cout << req.command.file_name << std::endl;
        if (req.command.type == CommandType::SERVER_PUSH) {
            cout << "push successful" << endl;
        }
    }
    else
    {
        cerr << "[ERROR] request failed: " << strerror(errno) << endl;
    }
}

void pull() {
    int viewserver_fd = server.connect(VIEWSERVER_IP, VIEWSERVER_PORT);
    std::thread(ping, viewserver_fd).detach();

    while (view.view_num == 0) {
        // busy wait
    }

    int colon_index = view.primary.find(":");
    std::string primary_ip = view.primary.substr(0, colon_index);
    int primary_port = std::stoi(view.primary.substr(colon_index + 1));

    int connected_fd = server.connect(primary_ip, primary_port);
    std::cout << "Client connected to " << primary_ip << ":" << primary_port << "\n";
    
    // send pull request
    ClientRequest request;
    request = ClientRequest(CommandType::CLIENT_PULL);
    server.send_message(request, connected_fd);
    
    // receive server response
    std::vector<char> in_buffer = std::vector<char>(1024);
    if (!server.receive_message(in_buffer.data(), connected_fd)) {
        std::cerr << "[ERROR] no response from server\n";
        return;
    }

    ClientReply reply;
    ClientReply::deserialize(in_buffer.data(), reply);

    if (reply.command.file_name.empty()) {
        std::cerr << "[ERROR] server returned empty file name\n";
        return;
    }

    // Ensure parent directories exist (supports nested paths in file_name)
    std::filesystem::path out_path(reply.command.file_name);
    if (!out_path.parent_path().empty()) {
        std::error_code ec;
        std::filesystem::create_directories(out_path.parent_path(), ec);
        if (ec) {
            std::cerr << "[ERROR] could not create directories for "
                      << out_path.parent_path().string() << ": " << ec.message() << "\n";
            return;
        }
    }

    // Open in binary + truncate to ALWAYS overwrite if it exists
    std::ofstream out(out_path, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!out) {
        std::cerr << "[ERROR] could not open " << reply.command.file_name << " for writing\n";
        return;
    }

    // Write bytes exactly as received
    if (!reply.command.data.empty())
        out.write(reply.command.data.data(), static_cast<std::streamsize>(reply.command.data.size()));
    out.close();

    std::cout << "Pulled " << reply.command.file_name << " (" << reply.command.data.size() << " bytes)\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "no gitd command provided" << endl;
        return 1;
    }

    string command = argv[1];

    if (command == "init") {
        init();
    } else if (command == "add") {
        if (argc < 3) {
            cout << "usage: gitd add [path]" << endl;
            return 1;
        }

        string file = argv[2];
        add(file);
    } else if (command == "commit") {
        commit();
    } else if (command == "push") {
        push();
    } else if (command == "pull") {
        pull();
    } else {
        cout << "'" << command << "' is not a git command" << endl;
        return 1;
    }

    return 0;
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
            std::cout << "view updated" << std::endl;
        }
        else
        {
            std::cerr << "[ERROR] request failed: " << strerror(errno) << std::endl;
        }
    }
}