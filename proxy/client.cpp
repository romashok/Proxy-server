#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cassert>

#include "socket_api.h"
#include "client.h"
#include "http/http_request_bodyless.h"
#include "http/http_request_with_body.h"


client_t::client_t(int fd):
    peer_t(fd),
    server(nullptr),
    request(nullptr)
{}

void client_t::read_request() {
    read();

    if (!request) {
        std::cout << "no request yet" << std::endl;
        size_t i = buffer.find("CONNECT");
        if (i != std::string::npos) {
            // todo catch this exception
            throw new std::runtime_error("http CONNECT is not implemented");
        }

        i = buffer.find("GET");
        if (i != std::string::npos) {
            std::cout << "new GET request" << std::endl;
            http_request* new_request = new (std::nothrow) http_request_bodyless(get_fd());
            if (!new_request) {
                std::cerr << "bad alloc" << std::endl;
                return;
            }
            request.reset(new_request);
        }

        // todo POST and PUT
        i = buffer.find("POST");
        if (i != std::string::npos) {
            std::cout << "new POST request" << std::endl;
            http_request* new_request = new (std::nothrow) http_request_with_body(get_fd());
            if (!new_request) {
                std::cerr << "bad alloc" << std::endl;
                return;
            }
            request.reset(new_request);
        }
    }

    if (request) {
        request->append_data(buffer);
        buffer.clear();
    }
}

size_t client_t::write_response(std::string const& msg) {
    return write(msg);
}

bool client_t::is_bad_request() const noexcept {
    assert(request);
    return request->is_bad_request();
}

bool client_t::has_data_to_send() const noexcept {
    return request->has_data_to_send();
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

//bool client_t::create_new_request() noexcept {
//    std::cout << "create new request" << std::endl;

//    if (buffer.substr()
//    http_request_stateful* new_request = new (std::nothrow) http_request_stateful(get_fd());
//    if (!new_request) return false;

//    request.reset(new_request);
//    return true;
//}

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
    assert(server);
    server->set_request(request.release());
}

int client_t::get_server_fd() {
    if (server) {
        return server->get_fd();
    } else {
        std::cout << "Client error: Get server fd error, no server." << std::endl;
        throw new std::runtime_error("No server");
    }
}

server_t* client_t::get_server() const noexcept {
    return server.get();
}
