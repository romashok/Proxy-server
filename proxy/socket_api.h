#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdexcept>
#include <iostream>
#include <sys/signalfd.h>
#include <signal.h>
#include <stdlib.h>

namespace socket_api {
    int create_socket(int domain, int type);

    void make_reusable(int fd);
    void make_non_blocking(int fd);

    void bind_socket(int fd, uint16_t port_net, uint32_t addr_net);
    void start_listen(int fd);
    void connect_socket(int fd, sockaddr addr);
    int accept(int fd);

    int create_main_socket(uint32_t port);
    int create_server_socket(sockaddr addr);
}
