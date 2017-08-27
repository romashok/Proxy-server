#pragma once

#include <map>
#include "sockets.h"
#include <sys/epoll.h>
#include <functional>


struct event_queue
{
    event_queue();

    void add_event(std::function<void(struct epoll_event&)> handler, int fd, uint32_t events);
//    add_event(struct epoll_event const& ev);
//    add_event(std::function<void(struct epoll_event const&)> handler, int fd, uint32_t events);
    void delete_event(struct epoll_event& ev);
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

    file_descriptor_t epoll;
    std::map<id,std::function<void(struct epoll_event&)>> handlers;

    static const size_t EVENTS_LIST_SIZE = 256;
    struct epoll_event events_list[EVENTS_LIST_SIZE];
};
