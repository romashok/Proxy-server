#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cassert>

#include "client.h"
#include "server.h"
#include "socket_api.h"
#include "http/http_response_plain.h"
#include "http/http_response_chunked.h"

server_t::server_t(int fd):
        peer_t(fd),
        client(nullptr),
        request(nullptr),
        response(nullptr)
{}

server_t::server_t(sockaddr addr):
        peer_t(socket_api::create_server_socket(addr)),
        client(nullptr),
        request(nullptr),
        response(nullptr)
{}

void server_t::bind(client_t* new_client) {
    client = new_client;
}

bool server_t::is_request_passed() const noexcept {
    return request->is_passed();
}

http_response const* server_t::get_response() {
    return response.get();
}

void server_t::set_request(http_request* new_request) {
    request.reset(new_request);
}

std::string server_t::get_host() const noexcept {
    return host;
}


int server_t::get_client_fd() {
    return client->get_fd();
}

void server_t::write_request() {
    assert(request);
    assert(!request->is_passed());

    if (request->has_data_to_send()) {
        std::string next = request->get_next_data_to_send();
        size_t delta = write(next);
        request->move_offset(delta);
    }
}

void server_t::read_response() {
    read();

    if (!response) {
        size_t i = buffer.find("Content-Length: ");
        if (i != std::string::npos) {
            std::cout << "new plain response" << std::endl;
            response.reset(new http_response_plain());
        }

        i = buffer.find("Transfer-Encoding: chunked");
        if (i != std::string::npos) {
            std::cout << "new chunked response" << std::endl;
            response.reset(new http_response_chunked());
        }
    }

    if (response) {
        response->append_data(get_buffer());
        buffer.clear();
    }
}

void server_t::move_response_offset(size_t delta) {
    response->move_offset(delta);
}
