#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/eventfd.h>

#include "host_resolver.h"

#define CACHE_SIZE 300

host_resolver::host_resolver():
    cache(CACHE_SIZE)
{
    evfd = eventfd(0, EFD_SEMAPHORE);
    if (evfd == -1) {
        perror("Can not create eventfd for resolver");
        return;
    }

    active = true;
    int i;
    try {
        for (i = 0; i < 4; ++i) {
            threads.push_back(std::thread([this]() {
                this->resolve();
            }));
        }
    } catch (...) {
        active = false;
        cond.notify_all();
        for (int j = 0; j < i; ++j) {
            if (threads[j].joinable())
                threads[j].join();
        }
    }
}

host_resolver::~host_resolver() {
    std::unique_lock<std::mutex> lock(mutex);
    active = false;
    cond.notify_all();
    lock.unlock();

    for (int i = 0; i < 4; ++i)
        if (threads[i].joinable())
            threads[i].join();
}


int host_resolver::get_eventfd() const noexcept {
    return evfd;
}

bool host_resolver::is_active() const noexcept {
    return active;
}

void host_resolver::push_host(std::tuple<int, std::string> cid_and_host) {
    std::lock_guard<std::mutex> lock(mutex);

    int client_fd;
    std::string host;
    std::tie (client_fd, host) = cid_and_host;
    std::cout << "pushed host {" << host << "} for client {" <<  client_fd << "}" << std::endl;

    auto it = cache.peek(host);
    if (it.first) {
        std::cout << "get from cache: " << host << std::endl;
        resolved.push(std::make_tuple(true, client_fd, host, it.second));
        notify_epoll();
        return;
    }

    pending.push(cid_and_host);
    cond.notify_one();
}

std::tuple<bool, int, std::string, sockaddr> host_resolver::pop_resolved_host() {
    std::lock_guard<std::mutex> lock(mutex);
    uint64_t tmp;
    int res;
    res = read(evfd, &tmp, sizeof(uint64_t));
    if (res != sizeof(uint64_t)) {
        std::cerr << "Error read from eventfd" << std::endl;
    }

    auto result = resolved.front();
    resolved.pop();
    return result;
}

void host_resolver::resolve() {
    while(true) {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [this] {
            return !pending.empty() || !active;
        });

        if (!active) break;

        int client_fd;
        std::string host;
        std::tie (client_fd, host) = pending.front();
        pending.pop();
        lock.unlock();

        size_t i = host.find(":");
        std::string port = "80";
        if (i != std::string::npos) {
            port = host.substr(i + 1);
        }
        std::string new_host_name = host.substr(0, i);

        struct addrinfo hints, *server_info;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(new_host_name.c_str(), port.c_str(), &hints, &server_info) != 0) {
            std::cout << std::flush;
            std::cerr << "RESOLVER: getaddrinfo error!" << std::endl;
            std::cerr << std::flush;
            // ATTENTION: UB in servaddr
            resolved.push(std::make_tuple(false, client_fd, host, *server_info->ai_addr));
            notify_epoll();
            return;
        }

        sockaddr server_addr = *server_info->ai_addr;
        freeaddrinfo(server_info);

        {
            std::lock_guard<std::mutex> lock(mutex);
            resolved.push(std::make_tuple(true, client_fd, host, server_addr));
            cache.push(host, server_addr);
            notify_epoll();
        }
    }
}

void host_resolver::notify_epoll() {
    uint64_t u = 1;
    int res = write(evfd, &u, sizeof(uint64_t));
    if (res == -1) {
        perror("Write to eventfd error");
    }
}
