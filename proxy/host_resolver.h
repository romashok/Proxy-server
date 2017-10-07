#pragma once

#include <queue>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <tuple>


#include "client.h"

struct client_t;

struct host_resolver {
    host_resolver();

    int get_eventfd() const noexcept;
    bool is_active() const noexcept;

    void resolve();

    void push_host(std::tuple<int, std::string> cid_and_host);
    std::tuple<int, std::string, sockaddr> pop_resolved_host();
    ~host_resolver();
private:
    bool active;

    std::queue<std::tuple<int, std::string>> pending;
    std::queue<std::tuple<int, std::string, sockaddr>> resolved;

    int evfd;
    std::mutex mutex;
    std::condition_variable cond;
    std::vector<std::thread> threads;

    std::tuple<int, std::string, sockaddr> push();
};
