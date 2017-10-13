#pragma once
#include <memory>

#include "socket_util.h"
#include "client.h"
#include "event_queue.h"
#include "http/http_request.h"

struct proxy_server
{
    proxy_server(uint32_t port);
    proxy_server(proxy_server const&)=delete;
    proxy_server& operator=(proxy_server const&)=delete;

    void run();

    void connect_client();
    void disconnect_client(int fd);
    void disconnect_server(int fd);

    void read_from_client(struct epoll_event& ev);
    void write_to_server(struct epoll_event& ev);
    void read_from_server(struct epoll_event& ev);
    void write_to_client(struct epoll_event& ev);

    void on_host_resolved(struct epoll_event& ev);
private:
    fd_t proxy_socket;
    event_queue queue;
    host_resolver resolver;
    bool is_working;

    std::map<uintptr_t, std::unique_ptr<client_t>> clients;
    std::map<uintptr_t, server_t*> servers;
};
