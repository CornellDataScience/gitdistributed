#include "viewserver.hpp"
#include "message.hpp"
#include <iostream>

constexpr int PING_INTERVAL_MS = 1000;
#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

std::unordered_map<std::string, int> serverToLastPinged;
View current_view;

TcpServer server(PORT, TcpMode::SERVER);
ViewServer viewserver;

ViewServer::ViewServer()
{
    running = true;
    pingCheckTimerThread = std::thread([this]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(PING_INTERVAL_MS));
            {
                std::unique_lock<std::mutex> pingCheckLock(viewserver.mtx);
                this->onPingCheckTimer();
            }
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

    serverToLastPinged[server_id] = 0;
    
    ViewReply view_reply;
    view_reply.view_num = current_view.view_num;
    view_reply.primary = current_view.primary;
    view_reply.backup = current_view.backup;
    return view_reply;
}

void ViewServer::onPingCheckTimer() {
    std::cout << "PING CHECK TIMER TRIGGERED" << std::endl;

    for (auto &pair : serverToLastPinged) {
        pair.second++;
    }
    if (primaryAcked) {
      std::vector<std::string> idleServers;
      
      for (auto const& pair : serverToLastPinged) {
          if (pair.first != current_view.primary && pair.first != current_view.backup && pair.second < 2) {
              idleServers.push_back(pair.first);
          }
      }
      bool viewChanged = false;
      std::string primary = current_view.primary;
      std::string backup = current_view.backup;
      
      // checking if the primary is not unassigned, and if >= 2 (has died)
      if (primary != "" && ((serverToLastPinged.count(primary) == 0 || serverToLastPinged[primary] >= 2))) {
          primary = backup;
          backup = "";
          viewChanged = true;
      }

      // checking analogously for backup...
      if (backup != "" && ((serverToLastPinged.count(backup) == 0) || serverToLastPinged[backup] >= 2)) {
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
}

void handle_connection(int connected_fd) {
    while (true)
    {
        char buffer[BUFFER_SIZE];
        
        bool received = server.receive_message(buffer, connected_fd);
        std::cout << received << std::endl;
        if (received)
        {
            Ping ping;
            std::cout << "Received client message" << std::endl;
            Ping::deserialize(buffer, ping);
            std::cout << "Deserialized message" << std::endl;
            
            ViewReply reply;
            {
                std::unique_lock<std::mutex> handlePingLock(viewserver.mtx);
                reply = viewserver.handlePing(ping.server_id, ping.view_num);
            }

            std::cout << "Handled request" << std::endl;
            server.send_message(reply, connected_fd);
            std::cout << "Sent response to client" << std::endl;
        }
        else
        {
            std::cerr << "[ERROR] recv failed: " << strerror(errno) << std::endl;
            return;
        }
    }
}

int main() {
    std::cout << "View Server listening on port " << PORT << "\n";

    while (true) {
        int connected_fd = server.connect(); // inside connect(), create new thread for each connection

        // for each connection, run the listening/responding loop
        std::thread(handle_connection, connected_fd).detach();
    }

    return 0;
}