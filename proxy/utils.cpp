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

void print_flags_extended() {
    std::cout << "EPOLLIN    " <<  EPOLLIN    << " " << events_to_str(EPOLLIN)    <<std::endl;
    std::cout << "EPOLLOUT   " <<  EPOLLOUT   << " " << events_to_str(EPOLLOUT)   <<std::endl;
    std::cout << "EPOLLERR   " <<  EPOLLERR   << " " << events_to_str(EPOLLERR)   <<std::endl;
    std::cout << "EPOLLHUP   " <<  EPOLLHUP   << " " << events_to_str(EPOLLHUP)   <<std::endl;
    std::cout << "EPOLLRDHUP " <<  EPOLLRDHUP << " " << events_to_str(EPOLLRDHUP) <<std::endl;
    std::cout << "EAGAIN     " <<  EAGAIN     << " " << events_to_str(EAGAIN)     <<std::endl;
}
void exit() {
        std::cout << "exit" << std::endl;
        std::exit(0);
}
