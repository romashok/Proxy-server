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
    peer_t(fd),
    server(nullptr),
    state(WAIT_REQUEST),
    request(nullptr)
{}

client_state client_t::get_state() const noexcept {
    return state;
}

void client_t::set_state(client_state new_state) {
    state = new_state;
}

bool client_t::has_request() const noexcept {
    return request.get() != nullptr;
}

bool client_t::create_request() {
    http_request* new_request = new (std::nothrow) http_request();
    if (!new_request) return false;

    request.reset(new_request);
    return true;
}

http_request* client_t::get_request() const noexcept {
    return request.get();
}

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
