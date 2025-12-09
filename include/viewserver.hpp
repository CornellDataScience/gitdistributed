#pragma once
#include "tcp.hpp"
#include <string>
#include <unordered_set>
#include <thread>
#include <atomic>
#include <mutex>

struct View
{
    int view_num = 0;
    // increment every time primary changes
    std::string primary;
    std::string backup;
};

class ViewServer
{
    public:
        ViewServer();
        ~ViewServer();
        ViewReply handlePing(const std::string server_id, int server_view_num);
    
    private:
        std::unordered_set<std::string> activeServers;
        View current_view;
        bool primaryAcked;
        
        
        void onPingCheckTimer();
        void send(const std::string server_id, const View view);

        std::thread pingCheckTimerThread;
        std::atomic<bool> running{false};
        std::mutex mtx;
};