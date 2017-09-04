#pragma once

#include <string>
#include <memory>

#include "socket_util.h"
#include "server.h"
#include "http_request.h"
#include "host_resolver.h"

struct http_request;

struct client_t : public peer_t {
    friend class host_resolver;

    client_t(int fd);

    size_t read_request();
    bool is_bad_request() const noexcept;
    bool is_ready() const noexcept;
    bool has_right_server() const noexcept;
    sockaddr get_server_addr();

    http_request* get_request() const noexcept;

    void bind(server_t* server);
    void unbind();
    bool has_server() const noexcept;

    std::string get_request_host() const noexcept;

    int get_server_fd();
    server_t* get_server() const noexcept;
    void move_request_to_server();
private:
    std::unique_ptr<struct server_t> server;
    std::unique_ptr<http_request> request;

    bool has_request() const noexcept;
    bool create_new_request() noexcept;
};
