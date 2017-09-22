#include "utils.h"

std::string events_to_str(uint32_t events) {
    std::string s;
    if (events & EPOLLIN) s.append(" EPOLLIN");
    if (events & EPOLLOUT) s.append(" EPOLLOUT");
    if (events & EPOLLERR) s.append(" EPOLLERR");
    if (events & EPOLLHUP) s.append(" EPOLLHUP");
    if (events & EPOLLRDHUP) s.append(" EPOLLRDHUP");
    if (events & EAGAIN) s.append(" EAGAIN");

    return s;
}

void print_flags() {
    std::cout << "EPOLLIN    " <<  EPOLLIN    << std::endl;
    std::cout << "EPOLLOUT   " <<  EPOLLOUT   << std::endl;
    std::cout << "EPOLLERR   " <<  EPOLLERR   << std::endl;
    std::cout << "EPOLLHUP   " <<  EPOLLHUP   << std::endl;
    std::cout << "EPOLLRDHUP " <<  EPOLLRDHUP << std::endl;
    std::cout << "EAGAIN     " <<  EAGAIN     << std::endl;
}

void exit() {
        std::cout << "exit" << std::endl;
        std::exit(0);
}
