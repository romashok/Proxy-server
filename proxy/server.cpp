#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "exceptions.h"
#include "sockets.h"
#include "client.h"
#include "server.h"

namespace socket_api {
    int create_socket(int domain, int type);
    void make_non_blocking(int fd);

    void connect_socket(int fd, sockaddr addr) {
        // TODO solve this nonblocking EINPROGRESS error
        if (connect(fd, &addr, sizeof(addr)) == -1 && errno != EINPROGRESS) {
            throw custom_exception("\nConnecting server error occurred!");
        }
    }

    int create_server_socket(sockaddr addr) {
        int fd = create_socket(AF_INET, SOCK_STREAM);
        make_non_blocking(fd);

        std::cout << "Connecting to IP: " << inet_ntoa(((sockaddr_in*) &addr)->sin_addr) << std::endl;
        std::cout << "Socket: " << fd << std::endl;

        connect_socket(fd, addr);
        return fd;
    }
}

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
