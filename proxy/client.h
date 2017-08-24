#pragma once
#include <string>
#include <memory>
#include <string>

#include "sockets.h"
#include "server.h"

struct client_t : peer_t {
    client_t(int fd);

    void bind(server_t* server);
    bool has_server() const noexcept;
    std::string get_request_host() const noexcept;

    int get_server_fd();
    void send_msg_to_server();
private:
    std::unique_ptr<struct server_t> server;
};
