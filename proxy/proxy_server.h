#pragma once

#include "sockets.h"
#include "event_queue.h"

struct proxy_server
{
    proxy_server(uint32_t port);
    proxy_server(proxy_server const&)=delete;
    proxy_server& operator=(proxy_server const&)=delete;

    void run();
private:
    file_descriptor_t proxy_socket;
    event_queue queue;
    bool is_working;
};
