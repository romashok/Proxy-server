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

    void make_reusable(int fd) {
        int enable = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
            throw custom_exception("Making reusable failed!");
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
        make_reusable(fd);
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

    queue.add_event([this](struct epoll_event& ev) {
        std::cout << "New connection!" << std::endl;
        this->connect_client(ev);
        std::cout << "Now " << clients.size() << "clients are connected!" << std::endl;
    }, proxy_socket.get_fd(), EPOLLIN);
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

void proxy_server::connect_client(epoll_event &ev) {
    client_t * new_client = new client_t(proxy_socket.get_fd());
    clients[new_client->get_fd()] = std::move(std::unique_ptr<client_t>(new_client));

    queue.add_event([this](struct epoll_event& ev) {
        this->read_from_client(ev);
    }, new_client->get_fd(), EPOLLIN);
}

void proxy_server::read_from_client(struct epoll_event& ev) {
    struct client_t* client = clients.at(ev.data.fd).get();

    client->read((int)BUFFER_SIZE - (int)client->get_buffer_size());
    std::cout << "Client data:[" << client->get_buffer() << "]" << std::endl;
}

