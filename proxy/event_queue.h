#pragma once

#include <sys/epoll.h>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <set>
#include <map>

#include "socket_util.h"

struct event_queue
{
    event_queue();

    void add_event(std::function<void(struct epoll_event&)> handler, int fd, uint32_t events);
    void delete_event(struct epoll_event& ev);
    void invalidate_event(int fd, uint32_t events);

    int get_events_amount();
    void handle_events(int amount);

    ~event_queue();

private:
    struct id {
        id();
        id(int fd, uint32_t events);
        id(struct epoll_event const& ev);

        friend bool operator==(id const&, id const&);
        friend bool operator<(id const&, id const&);
    private:
        int fd;
        uint32_t events;
    };
    friend bool operator==(id const&, id const&);
    friend bool operator<(id const&, id const&);

    fd_t epoll;
    std::map<id,std::function<void(struct epoll_event&)>> handlers;

    static const size_t EVENTS_LIST_SIZE = 256;
    struct epoll_event events_list[EVENTS_LIST_SIZE];

    std::set<id> invalid_events;
};
