#include "viewserver.hpp"
#include "message.hpp"
#include <iostream>

constexpr int PING_INTERVAL_MS = 1000;
#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

std::unordered_set<std::string> activeServers;
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

ViewReply ViewServer::handlePing(const std::string server_id, int server_view_num) {
    std::lock_guard<std::mutex> lock(mtx);
    // initialization of view_nums and primary + backup
    if (current_view.view_num == 0) {
        current_view = View{1, server_id, ""};
    }
    if (primaryAcked && server_id != current_view.primary && current_view.backup == "") {
        current_view = View{current_view.view_num + 1, current_view.primary, server_id};
    }

    if (server_id == current_view.primary && server_view_num == current_view.view_num) {
        primaryAcked = true;
    }

    activeServers.insert(server_id);
    
    ViewReply view_reply;
    view_reply.view_num = current_view.view_num;
    view_reply.primary = current_view.primary;
    view_reply.backup = current_view.backup;
    return view_reply;
}

void ViewServer::onPingCheckTimer() {
    std::lock_guard<std::mutex> lock(mtx);

    if (primaryAcked) {
      std::vector<std::string> idleServers;
      
      for (auto const& s : activeServers) {
          if (s != current_view.primary && s != current_view.backup) {
              idleServers.push_back(s);
          }
      }
      bool viewChanged = false;
      std::string primary = current_view.primary;
      std::string backup = current_view.backup;
      
      // checking if the primary is not unassigned and if primary is not active
      if (primary != "" && (activeServers.count(primary) == 0)) {
          primary = backup;
          backup = "";
          viewChanged = true;
      }

      // checking analogously for backup...
      if (backup != "" && (activeServers.count(backup) == 0)) {
        backup = "";
        viewChanged = true;
      }
    
      // promote idle server to replace backup
      if (backup == "" && !idleServers.empty()) {
          backup = idleServers[0];
          viewChanged = true;
      }
      // if the view change 
      if (viewChanged) {
          current_view = View{current_view.view_num + 1, primary, backup};
          primaryAcked = false;
      }
    }
    activeServers.clear();
}

int main() {
    TcpServer server(PORT, TcpMode::SERVER);
    ViewServer viewserver;
    while (true)
    {
        std::cout << "View Server listening on port " << PORT << "\n";

        char buffer[BUFFER_SIZE];

        server.connect();
        
        bool received = server.receive_message(buffer);
        std::cout << received << std::endl;
        if (received)
        {
            Ping ping;
            std::cout << "Received client message" << std::endl;
            Ping::deserialize(buffer, ping);
            std::cout << "Deserialized message" << std::endl;
            ViewReply reply = viewserver.handlePing(ping.server_id, ping.view_num);
            std::cout << "Handled request" << std::endl;
            server.send_message(reply, ping.server_id);
            std::cout << "Sent response to client" << std::endl;
        }
        else
        {
            std::cerr << "[ERROR] recv failed: " << strerror(errno) << std::endl;
            return 1;
        }
    }

    return 0;
}