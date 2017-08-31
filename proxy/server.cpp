#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "exceptions.h"
#include "client.h"
#include "server.h"

#include "socket_api.h"

server_t::server_t(int fd):
        peer_t(fd),
        client(nullptr)
{}

server_t::server_t(sockaddr addr):
        peer_t(socket_api::create_server_socket(addr)),
        client(nullptr)
{}

void server_t::bind(client_t* new_client) {
    client = new_client;
}

void server_t::set_host(std::string const& request_host) {
    host = request_host;
}

std::string server_t::get_host() const noexcept {
    return host;
}


int server_t::get_client_fd() {
    return client->get_fd();
}

void server_t::sent_msg_to_client() {
    client->append_to_buffer(buffer);
    buffer.clear();
}
