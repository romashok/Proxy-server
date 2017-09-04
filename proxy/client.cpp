#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "exceptions.h"
#include "socket_api.h"
#include "client.h"
#include "http_request.h"


client_t::client_t(int fd):
    peer_t(fd),
    server(nullptr),
    request(nullptr)
{}

size_t client_t::read_request() {
    read();

    if (!has_request() && !create_new_request()) {
        std::cout << "Allocation problems for new request!";
        return 0;
    }

    request->append_data(get_buffer());
    buffer.clear();
}

bool client_t::is_bad_request() const noexcept {
    return !request || request->get_state() == BAD_REQUEST;
}

bool client_t::is_ready() const noexcept {
    return request->get_state() == COMPLETED;
}


bool client_t::has_right_server() const noexcept {
    return server && server->get_host() == request->get_host();
}

sockaddr client_t::get_server_addr() {
    return std::move(request->get_server_addr());
}

bool client_t::has_request() const noexcept {
    return request.get() != nullptr;
}

bool client_t::create_new_request() noexcept {
    std::cout << "create new request" << std::endl;
    http_request* new_request = new (std::nothrow) http_request(get_fd());
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

// can be safty used for client without the server
void client_t::unbind() {
    // todo better clearing
    server.reset(nullptr);
}

bool client_t::has_server() const noexcept {
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

void client_t::move_request_to_server() {
    if (has_server()) {
        server->set_request(request.release());
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

server_t* client_t::get_server() const noexcept {
    return server.get();
}
