#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "exceptions.h"
#include "sockets.h"
#include "client.h"
#include "server.h"


server_t::server_t(int fd):
        peer_t(fd),
        client(nullptr)
{}

void server_t::bind(client_t* new_client) {
    client = new_client;
}
