#include "viewserver.hpp"
#include <iostream>

constexpr int PING_INTERVAL_MS = 1000;
#define PORT 8080
#define SERVER_IP "127.0.0.1"

std::unordered_map<std::string, int> serverToLastPinged;
View current_view;

ViewServer::ViewServer()
{
    running = true;
    pingCheckTimerThread = std::thread([this]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(PING_INTERVAL_MS));
            this->onPingCheckTimer();
        }
    });
}

ViewServer::~ViewServer()
{
    running = false;
    if (pingCheckTimerThread.joinable())
        pingCheckTimerThread.join();
}

int main() {
    //
}

View ViewServer::handlePing(const std::string server_id) {
    std::lock_guard<std::mutex> lock(mtx);
    if (current_view.view_num == 0) {
        current_view = View{1, server_id, ""};
    }
    
    serverToLastPinged[server_id] = 0;
    
    // Command view_command = ;
    // server.send_message(view_command, server_id);
}

void ViewServer::onPingCheckTimer() {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto const& pair : serverToLastPinged) {
        serverToLastPinged[pair.first] += 1;
        if (serverToLastPinged[pair.first] >= 2) {
            serverToLastPinged.erase(pair.first);
        }
    }
}