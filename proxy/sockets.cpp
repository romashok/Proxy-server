#include <iostream>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "exceptions.h"
#include "sockets.h"


file_descriptor_t::file_descriptor_t():
    fd(-1)
{}

file_descriptor_t::file_descriptor_t(int fd):
    fd(fd)
{}

file_descriptor_t::file_descriptor_t(file_descriptor_t&& rhs):
    fd(rhs.get_fd())
{
    rhs.set_fd(-1);
}

file_descriptor_t& file_descriptor_t::operator=(file_descriptor_t&& rhs) {
    fd = rhs.get_fd();
    rhs.set_fd(-1);

    return *this;
}

void file_descriptor_t::set_fd(int new_fd) {
    fd = new_fd;
}

int file_descriptor_t::get_fd() const {
    return fd;
}

file_descriptor_t::~file_descriptor_t() {
    if (fd == -1) {
        return;
    }

    if (close(fd) == -1){
        perror("During closing file descriptor some error occured!");
    }
    std::cout << "File descriptor: " << fd << " closed." << std::endl;
}

socket_t::socket_t(int fd):
    file_descriptor_t(fd)
{}

socket_t::~socket_t()
{}

socket_t socket_t::accept(int accept_fd) {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    int fd = ::accept(accept_fd, (sockaddr*) &addr, &len);
    if (fd == -1) {
        throw custom_exception("New client connection error occured!");
    }
    std::cout << "New client accepted to fd[" << fd << "]." << std::endl;


    return socket_t(fd);
}

std::string socket_t::read(size_t buffer_size) {
    std::vector<char> buffer(buffer_size);
    size_t length = recv(fd, buffer.data(),buffer_size, 0);

    //todo NON_BLOCK
    if (length == -1)
        throw custom_exception("Read from socket error occured!");

    return std::string(buffer.cbegin(), buffer.cbegin() + length);
}


size_t socket_t::write(std::string const& msg) {
    size_t length = send(fd, msg.data(), msg.length(), 0);

    //todo NON_BLOCK
    if (length == -1)
        throw custom_exception("Write to socket error occured!");

    return length;
}

//=============== PEER ==============
peer_t::peer_t(int fd):
        socket(socket_t::accept(fd))
{}

int peer_t::get_fd() const {
    return socket.get_fd();
}

std::string& peer_t::get_buffer() {
    return buffer;
}

void peer_t::append_to_buffer(std::string &s) {
    buffer.append(s);
}

size_t peer_t::get_buffer_size() const {
    return buffer.size();
}


bool peer_t::is_full_buffer() const {
    return buffer.size() >= BUFFER_SIZE;
}

size_t peer_t::read(size_t size) {
    try {
        std::string s{socket.read(size)};
        buffer.append(s);
        return s.size();
    } catch (...) {
        return 0;
    }
}

