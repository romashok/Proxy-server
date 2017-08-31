#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "exceptions.h"
#include "socket_api.h"
#include "client.h"


client_t::client_t(int fd):
    peer_t(socket_api::accept(fd)),
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

std::string client_t::get_request_host() const noexcept {
    if (has_server()) {
        return server->get_host();
    } else {
        std::cerr << "Client has no server.";
        return nullptr;
    }
}

void client_t::send_msg_to_server() {
    if (has_server()) {
        server->append_to_buffer(buffer);
        buffer.clear();
    } else {
        std::cout << "Client error: No server to write!" << std::endl;
    }
}

int client_t::get_server_fd() {
    if (server) {
        return server->get_fd();
    } else {
        std::cout << "Client error: Get server fd error, no server." << std::endl;
        throw new custom_exception("No server");
    }
}
