#include <iostream>
#include <filesystem>
#include <fstream>
#include "tcp.hpp"
#include "commands.hpp"
#include "message.hpp"

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

using namespace std;
using namespace std::filesystem;

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
    TcpServer server(PORT, TcpMode::CLIENT);
    int connected_fd = server.connect(SERVER_IP, 8080);
    std::cout << "Client connected to port " << PORT << "\n";

    // ClientRequest(CommandType cmd, std::string name, std::vector<char> data = {});
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

    std::vector<char> buffer = std::vector<char>(1024);
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
    TcpServer client(PORT, TcpMode::CLIENT);
    std::cout << "Server listening on port " << PORT << "\n";    
    int connected_fd = client.connect();
    
    // send pull request
    ClientRequest request;
    request = ClientRequest(CommandType::CLIENT_PULL);
    client.send_message(request, connected_fd);
    
    // receive server response
    std::vector<char> in_buffer = std::vector<char>(1024);
    if (!client.receive_message(in_buffer.data(), connected_fd)) {
        std::cerr << "[ERROR] no response from server\n";
        return;
    }

    Command resp = deserializeCommand(in_buffer.data());

    if (resp.file_name.empty()) {
        std::cerr << "[ERROR] server returned empty file name\n";
        return;
    }

    // Ensure parent directories exist (supports nested paths in file_name)
    std::filesystem::path out_path(resp.file_name);
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
        std::cerr << "[ERROR] could not open " << resp.file_name << " for writing\n";
        return;
    }

    // Write bytes exactly as received
    if (!resp.data.empty())
        out.write(resp.data.data(), static_cast<std::streamsize>(resp.data.size()));
    out.close();

    std::cout << "Pulled " << resp.file_name << " (" << resp.data.size() << " bytes)\n";
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