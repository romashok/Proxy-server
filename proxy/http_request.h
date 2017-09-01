#pragma once

#include <string>

#include "socket_util.h"
#include "client.h"

enum request_state {BAD_REQUEST, // must send bad_request to client
                    HEADER,      // getting request header
                    BODY,        // getting request body
                    COMPLETED,   // got comleted request
                    RESOLVING,   // resolving request host
                    RESOLVED};   // request host resoled

struct http_request {
    http_request(int fd);

    request_state get_state() const noexcept;
    void set_state(request_state new_state) noexcept;


    void append_data(std::string const& str);
    std::string get_raw_text() const noexcept;

    // getters and setters
    std::string get_host() const noexcept;

    int get_client_fd() const noexcept;

    void set_server_addr(struct sockaddr addr);
    sockaddr get_server_addr() const noexcept;
private:
    request_state state;

    std::string header, body;
    std::string host, url;

    size_t content_length;

    int client_fd;
    struct sockaddr server_addr;

    // utils
    bool is_valid_host();
    void ensure_relative_url();
    bool if_post_or_put();
};

