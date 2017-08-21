#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "exceptions.h"
#include "client.h"


client_t::client_t(int fd):
    peer_t(fd),
    server(nullptr)
{}


void client_t::bind(struct server_t* new_server) {
    server.reset(new_server);
    server->bind(this);
}

bool client_t::has_server() const noexcept {
    // todo unique_ptr op::bool
    return server.get() != nullptr;
}
