#pragma once
#include <memory>

#include "sockets.h"
#include "client.h"
#include "event_queue.h"
#include "http_request.h"


struct proxy_server
{
    proxy_server(uint32_t port);
    proxy_server(proxy_server const&)=delete;
    proxy_server& operator=(proxy_server const&)=delete;

    void run();

    void connect_client();
    void disconnect_client(struct epoll_event& ev);

    void read_from_client(struct epoll_event& ev);
private:
    file_descriptor_t proxy_socket;
    event_queue queue;
    bool is_working;

    std::map<uintptr_t, std::unique_ptr<client_t>> clients;
    std::map<uintptr_t, server_t*> servers;

    void resolve(http_request *requst);
};
