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

server_t::server_t(int fd):
        peer_t(fd),
        client(nullptr),
        request(nullptr),
{}

server_t::server_t(sockaddr addr):
        peer_t(socket_api::create_server_socket(addr)),
        client(nullptr),
        request(nullptr),
{}

void server_t::bind(client_t* new_client) {
    client = new_client;
}

bool server_t::is_sent_all_request() const noexcept {
    return offset == request->get_raw_text().size();
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
    size_t length = write(get_next_part());
    offset += length;
}

std::string server_t::get_next_part() const {
    assert(request);
    if (offset == request->get_raw_text().size()) return "";
    // todo ensure request status quering
    std::cout << "offset: " << offset  << " size: " << request->get_raw_text().size() << std::endl;
    return request->get_raw_text().substr(offset);
}

}
