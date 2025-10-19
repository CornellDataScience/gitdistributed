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
    
    std::cout << "Server listening on port " << PORT << "\n";
    char buffer[BUFFER_SIZE] = {0};
    server.connect();


    Message msg;
    msg.type = MessageType::CLIENT_PUSH;

}

void pull() {
    TcpServer client(PORT, TcpMode::CLIENT);
    std::cout << "Server listening on port " << PORT << "\n";    
    client.connect();
    
    char out_buffer[BUFFER_SIZE] = {0};
    char *p = out_buffer;
    Message m;
    m.type = MessageType::CLIENT_PULL;
    client.send_message(m);

    char in_buffer[BUFFER_SIZE] = {0};
    if (!client.receive_message(inbuf)) {
        std::cerr << "[ERROR] no response from server\n";
        return;
    }

    Message resp = deserialize(in_buffer);

    ofstream out(resp.file_name, ios::binary);
    if (!out) {
        cerr << "[ERROR] could not open " << resp.file_name << " for writing\n";
        return;
    }
    out.write(resp.data.data(), static_cast<streamsize>(resp.data.size()));
    out.close();

    cout << "Pulled " << resp.file_name << " (" << resp.data.size() << " bytes)\n";
}