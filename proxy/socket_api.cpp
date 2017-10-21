#include "socket_api.h"
#include <signal.h>
namespace socket_api {
    int create_socket(int domain, int type) {
        int fd = ::socket(domain, type, 0);
        if (fd == -1) {
            perror("socket_api::create_socket");
            throw std::runtime_error("Creat socket failed!");
        }

        return fd;
    }

    void make_reusable(int fd) {
        int enable = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
            perror("socket_api::make_reusable");
            throw std::runtime_error("Make socket reusable failed!");
        }
    }

    void make_non_blocking(int fd) {
        if (::fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
            perror("socket_api::make_non_blocking");
            throw std::runtime_error("Set socket nonblocking failed!");
        }
    }

    void bind_socket(int fd, uint16_t port_net, uint32_t addr_net) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = port_net;
        addr.sin_addr.s_addr = addr_net;

        if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("socket_api::bind_socket");
            throw std::runtime_error("Bind socket failed!");
        }
    }

    void start_listen(int fd) {
        if (::listen(fd, SOMAXCONN) == -1) {
            perror("socket_api::start_listen");
            throw std::runtime_error("Filled to start listen socket on fd(" + std::to_string(fd) + ").");
        }
    }

    void connect_socket(int fd, sockaddr addr) {
        // TODO solve this nonblocking EINPROGRESS error
        if (connect(fd, &addr, sizeof(addr)) == -1 && errno != EINPROGRESS) {
            perror("socket_api::connect_socket");
            throw std::runtime_error("Connect socket failed!");
        }
    }

    int accept(int accept_fd) {
        sockaddr_in addr;
        socklen_t len = sizeof(addr);

        int new_fd = ::accept(accept_fd, (sockaddr*) &addr, &len);
        if (new_fd == -1) {
            perror("socket_api::accept");
            throw std::runtime_error("New client connection failed!");
        }

        std::cout << "New client accepted to fd[" << new_fd << "]" << std::endl;
        return new_fd;
    }

    int create_main_socket(uint32_t port) {
        int fd = create_socket(AF_INET, SOCK_STREAM);
        make_reusable(fd);
        bind_socket(fd, htons(port), INADDR_ANY);
        make_non_blocking(fd);
        start_listen(fd);
        return fd;
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
