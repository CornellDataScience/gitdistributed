#pragma once
#include "tcp.hpp"
#include <string>
#include <unordered_map>
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
    std::unordered_map<std::string, int> serverToLastPinged;
    View current_view;
    
    View handlePing(const std::string server_id);
    void onPingCheckTimer();
    void send(const std::string server_id, const View view);

    std::thread pingCheckTimerThread;
    std::atomic<bool> running{false};
    std::mutex mtx;
};