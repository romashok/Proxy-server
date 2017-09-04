#include "utils.h"

std::string events_to_str(uint32_t events) {
    std::string s;
    if (events & EPOLLIN) s.append(" EPOLLIN");
    if (events & EPOLLOUT) s.append(" EPOLLOUT");
    if (events & EPOLLERR) s.append(" EPOLLERR");
    if (events & EPOLLHUP) s.append(" EPOLLHUP");
    if (events & EPOLLERR) s.append(" EPOLLERR");

    return s;
}
