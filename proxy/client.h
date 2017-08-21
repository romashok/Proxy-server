#pragma once
#include <string>
#include <memory>

#include "sockets.h"
#include "server.h"

struct client_t : peer_t {
    client_t(int fd);

    void bind(server_t* server);
    bool has_server() const noexcept;
private:
    std::unique_ptr<struct server_t> server;
};
