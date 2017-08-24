#pragma once
#include <string>

#include "client.h"

struct server_t : peer_t {
    server_t(int fd);
    server_t(sockaddr addr);

    void bind(struct client_t* client);

    void set_host(std::string const& host);
    std::string get_host() const noexcept;
private:
    std::string host;
    struct client_t* client;
};
