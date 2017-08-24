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

#define CONNECTION_TIMEOUT 3

namespace socket_api {
    int create_socket(int domain, int type);
    void make_non_blocking(int fd);

    void connect_socket(int fd, sockaddr addr) {
        // TODO TODO TODO solve this nonblocking EINPROGRESS error
        std::cout << "start conn" << std::endl;
        if (connect(fd, &addr, sizeof(addr)) == -1) {
            if (errno != EINPROGRESS) {
                throw custom_exception("\nConnecting error occurred!");
            }
        }
    }

    int create_server_socket(sockaddr addr) {
        std::cout << "create" << std::endl;
        int fd = create_socket(AF_INET, SOCK_STREAM);

        std::cout << "non block" << std::endl;
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
