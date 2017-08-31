#pragma once

#include <string>

#include "socket_util.h"
#include "client.h"

struct http_request {
    http_request(std::string& str);
    http_request(const http_request& request);

    static bool is_complete_request(const std::string& str);

    std::string get_host() const noexcept;

    void set_client_fd(int fd);
    int get_client_fd() const noexcept;
    void set_server_addr(struct sockaddr addr);
    sockaddr get_server_addr();

    bool is_resolved() const noexcept;
    void set_resolved(bool state) noexcept;
private:
    std::string header;
    std::string host;
    std::string path;

    int client_fd;
    struct sockaddr server_addr;
    bool resolve_state;

    void parse();
    bool has_host();
    void extract_relative_url();
};

