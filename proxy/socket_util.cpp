#include <iostream>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "exceptions.h"
#include "socket_util.h"


// ================ FD ================

fd_t::fd_t():
    fd(INVALID_FD)
{}

fd_t::fd_t(int fd):
    fd(fd)
{}

fd_t::fd_t(fd_t&& rhs):
    fd(rhs.get_fd())
{
    rhs.set_fd(INVALID_FD);
}

fd_t& fd_t::operator=(fd_t&& rhs) {
    fd = rhs.get_fd();
    rhs.set_fd(INVALID_FD);

    return *this;
}

void fd_t::set_fd(int new_fd) {
    fd = new_fd;
}

int fd_t::get_fd() const noexcept {
    return fd;
}

fd_t::~fd_t() {
    if (fd == INVALID_FD) {
        return;
    }

    if (close(fd) == -1) {
        perror("Close fd failed!");
    } else {
        std::cout << "File descriptor: " << fd << " closed." << std::endl;
    }
}

// ================ SOCKET ================

socket_t::socket_t(int fd):
    fd_t(fd)
{}

socket_t::~socket_t()
{}

std::string socket_t::read(size_t buffer_size) {
    std::vector<char> buffer(buffer_size);
    size_t length = recv(fd, buffer.data(), buffer_size, 0);

    if (length == -1)
        throw std::runtime_error("Read from socket failed!");

    return std::string(buffer.cbegin(), buffer.cbegin() + length);
}


size_t socket_t::write(std::string const& msg) {
    size_t length = send(fd, msg.data(), msg.size(), 0);

    if (length == -1)
        throw std::runtime_error("Write to socket failed!");

    return length;
}

// ================ PEER ================
peer_t::peer_t(int fd):
        socket(fd)
{}

int peer_t::get_fd() const  noexcept {
    return socket.get_fd();
}

std::string& peer_t::get_buffer() {
    return buffer;
}

void peer_t::append_to_buffer(std::string &s) {
    buffer.append(s);
}

size_t peer_t::get_buffer_size() const noexcept {
    return buffer.size();
}

bool peer_t::is_full_buffer() const noexcept {
    return buffer.size() >= BUFFER_SIZE;
}

bool peer_t::is_empty_buffer() const noexcept {
    return buffer.empty();
}

size_t peer_t::read() {
    try {
        std::string s{socket.read(BUFFER_SIZE)};
        buffer.append(s);
        return s.size();
    } catch (...) {
        return 0;
    }
}

size_t peer_t::write() {
    try {
        size_t len = socket.write(buffer);
        buffer = buffer.substr(len);
        return len;
    } catch (...) {
        return 0;
    }
}
