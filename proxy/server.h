#pragma once
#include <string>

#include "client.h"
#include "http/http_request.h"
#include "http/http_response.h"

struct http_request;

struct server_t : peer_t {
    server_t(int fd);
    server_t(sockaddr addr);

    void bind(struct client_t* client);
    void set_request(http_request* new_request);
    bool is_request_passed() const noexcept;
    http_response const* get_response();

    void set_host(std::string const& host);
    std::string get_host() const noexcept;

    int get_client_fd();
    void write_request();
    void read_response();

    void move_response_offset(size_t delta);
private:
    std::string host;
    struct client_t* client;
    std::unique_ptr<http_request> request;
    std::unique_ptr<http_response> response;

    std::string get_next_part() const;
};
