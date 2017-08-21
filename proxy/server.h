#pragma once
#include <string>

#include "client.h"

struct server_t : peer_t {
    server_t(int fd);

    void bind(struct client_t* client);
private:
    struct client_t* client;
};
