#include "event_queue.h"
#include "exceptions.h"
#include "sockets.h"
#include <sys/epoll.h>

// legacy
#define EXPECTED_CLIENTS 100

namespace {
    int create_epollfd() {
        int fd = epoll_create(EXPECTED_CLIENTS);

        if (fd == -1)
            throw custom_exception("During creating epollfd error occured!");

        return fd;
    }
}


event_queue::event_queue():
    epoll(create_epollfd())
{}

void event_queue::add_event(std::function<void(struct epoll_event&)> handler, int fd, uint32_t events) {
    std::cout << "Add new event to queue." << std::endl;
    handlers[id{fd, events}] = handler;

    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;

    if (epoll_ctl(epoll.get_fd(), EPOLL_CTL_ADD, fd, &ev) == -1) {
        std::cout << "Failed to add new event on fd(" << std::to_string(fd).c_str() << ") to epoll." << std::endl;
        perror("Failed to add new event to epoll.");
    }
}

void event_queue::handle_events() {
    int epoll_events_count = epoll_wait(epoll.get_fd(), events_list, EVENTS_LIST_SIZE, -1);

    if (epoll_events_count == -1) {
        perror("Epoll wait error!");
    }

    for (int i = 0; i < epoll_events_count; ++i) {
        std::function<void(struct epoll_event&)> handler = handlers[id{events_list[i]}];
        handler(events_list[i]);
    }
}

event_queue::~event_queue()
{}

event_queue::id::id():
    ident(0),
    events(0)
{}

event_queue::id::id(int ident, uint32_t events):
    ident(ident),
    events(events)
{}

event_queue::id::id(const epoll_event &ev) :
    ident(ev.data.fd),
    events(ev.events)
{}

bool operator==(event_queue::id const& lhs, event_queue::id const& rhs) {
    return lhs.ident == rhs.ident && lhs.events == rhs.events;
}

bool operator<(event_queue::id const& lhs, event_queue::id const& rhs) {
    return lhs.ident <  rhs.ident || (lhs.ident == rhs.ident && lhs.events < rhs.events);
}

