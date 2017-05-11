#include <iostream>
#include <unistd.h>

#include "sockets.h"


file_descriptor_t::file_descriptor_t():
    fd(-1)
{}

file_descriptor_t::file_descriptor_t(int fd):
    fd(fd)
{}

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
    std::cout << "File descriptor: " << fd << " closed.\n";
}
