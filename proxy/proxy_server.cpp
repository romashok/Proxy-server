#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "exceptions.h"
#include "proxy_server.h"

namespace socket_api {
    int create_socket(int domain, int type) {
        int fd = ::socket(domain, type, 0);
        if (fd == -1)
            throw custom_exception("Creation of a socket failed!");

        return fd;
    }

    void bind_socket(int fd, uint16_t port_net, uint32_t addr_net) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = port_net;
        addr.sin_addr.s_addr = addr_net;

        if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) == -1)
            throw custom_exception("During binding error occured!");
    }

    void make_non_blocking(int fd) {
        if (::fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
            throw custom_exception("Setting nonblocking failed!");
    }

    void start_listen(int fd) {
        if (::listen(fd, SOMAXCONN) == -1)
            throw custom_exception("Filled to start listen socket on fd(" + std::to_string(fd) + ").");
    }

    int make_main_socket(uint32_t port) {
        int fd = create_socket(AF_INET, SOCK_STREAM);
        bind_socket(fd, htons(port), INADDR_ANY);
        make_non_blocking(fd);
        start_listen(fd);
        return fd;
    }

}

proxy_server::proxy_server(uint32_t port):
    proxy_socket(socket_api::make_main_socket(port)),
    queue(),
    is_working(true)
{
    std::cout << "Main socket established on fd[" << proxy_socket.get_fd() << "]." << std::endl;

    queue.add_event([this](struct epoll_event& ev){std::cout << "Connect attemption!\n";},
                proxy_socket.get_fd(), EPOLLIN);
}


void proxy_server::run() {
    std::cout << "Start proxy server!" << std::endl;

    try{
        while(is_working) {
            int amount = queue.get_events_amount();

            if (amount == -1)
                perror("Error occured during getting new epoll events!");

            queue.handle_events(amount);
        }
    } catch (custom_exception& e) {
        std::cout << e.what();
    }
}

