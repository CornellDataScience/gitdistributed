#include <iostream>
#include <filesystem>

using namespace std;
using namespace std::filesystem;

void init();
void add(string name);

int main() {
    cout << "Hello world" << endl;
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