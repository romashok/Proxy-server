#pragma once

#include <string>

#include "socket_util.h"
#include "client.h"

enum request_state {BAD_REQUEST, HEADER, BODY, COMPLETED, RESOLVING, RESOLVED, REQUESTING};

struct http_request {
    http_request(int fd);
//    http_request(const http_request& request);

    request_state get_state() const noexcept;
    bool is_completed();
    void append_data(std::string const& str);

    std::string get_host() const noexcept;

//    void set_client_fd(int fd);
    int get_client_fd() const noexcept;
//    void set_server_addr(struct sockaddr addr);
//    sockaddr get_server_addr();

//    bool is_resolved() const noexcept;
//    void set_resolved(bool state) noexcept;

private:
    request_state state;

    std::string header;
    std::string body;

    std::string host, url;
//    std::string path;

    size_t content_length;

    int client_fd;
//    struct sockaddr server_addr;
//    bool resolve_state;

//    void parse();
//    bool has_host();

    bool if_post_or_put();
    bool is_valid_host();
    void ensure_relative_url();
};

