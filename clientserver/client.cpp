#include <iostream>
#include <filesystem>
#include "tcp.hpp"
#include "messages.hpp"

#define PORT 8080
#define SERVER_IP ""
#define BUFFER_SIZE 1024

using namespace std;
using namespace std::filesystem;

void init();
void add(string name);
void commit();

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
    server.connect();
    std::cout << "Client connected to port " << PORT << "\n";

    Message msg;
    msg.type = MessageType::CLIENT_PUSH;

    const path commitspath = ".gitd/commits/";

    // Assuming only one file in commitsfolder
    for (const auto& file : directory_iterator(commitspath)) {
        try {
            msg.file_name = file.path().filename().string();
            std::cout << "Found file: " << filename << std::endl;

            // Open the file
            std::ifstream file(entry.path());

            // Check if the file opened successfully
            if (file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
                msg.data = content;
            } else {
                std::cerr << "Failed to open file." << std::endl;
            }
        } catch (const filesystem_error& e) {
            cout << "error reading file " << file.path() << ": " << e.what() << endl;
        }
    }

    // TODO: handle deleting from commits folder

    // make continuous requests
    server.send_message(msg);

    char buffer[BUFFER_SIZE] = {0};
    bool received = server.receive_message(buffer);
    if (received)
    {
        Message req = deserialize(buffer);
        if (req.type == MessageType::SERVER_PUSH) {
            cout << "push successful" << endl;
        }
    }
    else
    {
        cerr << "[ERROR] request failed: " << strerror(errno) << endl;
    }
}

void pull() {
    TcpServer server(PORT, TcpMode::SERVER);
    std::cout << "Server listening on port " << PORT << "\n";
    char buffer[BUFFER_SIZE] = {0};
    server.connect();

}