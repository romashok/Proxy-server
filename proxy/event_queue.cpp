#include "event_queue.h"
#include "socket_util.h"
#include <sys/epoll.h>

// legacy
#define EXPECTED_CLIENTS 100

namespace {
    int create_epollfd() {
        int fd = epoll_create(EXPECTED_CLIENTS);

        if (fd == -1) {
            perror("create_epollfd");
            throw std::runtime_error("Create epollfd failed!");
        }

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
        perror("Failed to add new event to epoll.");
        std::cout << "Failed to add new event on fd(" << std::to_string(fd).c_str() << ") to epoll." << std::endl;
    }
}

void event_queue::delete_event(struct epoll_event& ev) {
    std::cout << "Delete fd [" << ev.data.fd << "] from epoll!" << std::endl;

    auto it = handlers.find(id{ev.data.fd, ev.events});
    if (it != handlers.end()) {
        handlers.erase(it);

        // legacy non-NULL ev param with EPOLL_CTL_DEL for kernel before 2.6.9
        if (epoll_ctl(epoll.get_fd(), EPOLL_CTL_DEL, ev.data.fd, &ev) < 0) {
            perror("Failed to delete event from epoll.");
            std::cout << "Failed to delete from epoll fd [" << ev.data.fd << "]" << std::endl;
        }
    }
}


void event_queue::invalidate_event(int fd, uint32_t events) {
    invalid_events.insert(id{fd, events});
}

int event_queue::get_events_amount() {
    int epoll_events_count = epoll_wait(epoll.get_fd(), events_list, EVENTS_LIST_SIZE, -1);

    if (epoll_events_count == -1) {
        perror("Epoll wait failed!");
    }

    return epoll_events_count;
}


void event_queue::handle_events(int amount) {
    invalid_events.clear();

    for (int i = 0; i < amount; ++i) {
        if (!invalid_events.count(id{events_list[i].data.fd, events_list[i].events})) {
            std::function<void(struct epoll_event&)> handler = handlers[id{events_list[i]}];
            handler(events_list[i]);
        }
    }
}

event_queue::~event_queue()
{}

event_queue::id::id():
    fd(0),
    events(0)
{}

event_queue::id::id(int fd, uint32_t events):
    fd(fd),
    events(events)
{}

event_queue::id::id(const epoll_event &ev) :
    fd(ev.data.fd),
    events(ev.events)
{}

bool operator==(event_queue::id const& lhs, event_queue::id const& rhs) {
    return lhs.fd == rhs.fd && lhs.events == rhs.events;
}

bool operator<(event_queue::id const& lhs, event_queue::id const& rhs) {
    return lhs.fd <  rhs.fd || (lhs.fd == rhs.fd && lhs.events < rhs.events);
}

