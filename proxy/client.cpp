#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "exceptions.h"
#include "sockets.h"
#include "client.h"


client_t::client_t(int fd):
        socket(socket_t::accept(fd))
{}

int client_t::get_fd() const {
    return socket.get_fd();
}

std::string& client_t::get_buffer() {
    return buffer;
}

void client_t::append_to_buffer(std::string &s) {
    buffer.append(s);
}

size_t client_t::get_buffer_size() const {
    return buffer.size();
}


bool client_t::is_full_buffer() const {
    return buffer.size() >= BUFFER_SIZE;
}

size_t client_t::read(size_t size) {
    try {
        std::string s{socket.read(size)};
        buffer.append(s);
        return s.size();
    } catch (...) {
        return 0;
    }
}
