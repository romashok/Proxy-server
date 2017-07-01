#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

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
