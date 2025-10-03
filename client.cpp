#include <iostream>
#include <filesystem>

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
    path folder = "./gitd";
    if (!exists(folder)) { create_directory(folder); }
    
    path subfolder1 = "./gitd/objects", subfolder2 = "./gitd/commits";
    if (!exists(subfolder1)) { create_directory(subfolder1); }
    if (!exists(subfolder2)) { create_directory(subfolder2); }
}

void add(string name) {
    copy(name, "gitd/objects" + name);
}

void commit() {
    const path gitdpath = ".gitd";

    if (!exists(gitdpath)) {
        cout << "not a gitd repository" << endl;
        return;
    }

    const path objectspath = ".gitd/objects";
    const path commitspath = ".gitd/commits";

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
