#include <sys/epoll.h>

#include "utils.h"
#include "event_queue.h"
#include "socket_util.h"
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

void event_queue::create_events(std::function<void(struct epoll_event&)> handler, int fd, uint32_t events) {
    std::cout << "Create event: fd= " << fd
              << " flags= " << events_to_str(events) << std::endl;
    handlers[id{fd, events}] = handler;

    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;

    if (epoll_ctl(epoll.get_fd(), EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("Failed to create event");
    }
}

void event_queue::merge_events(std::function<void(struct epoll_event&)> handler, int fd, uint32_t new_events, uint32_t old_events) {
    std::cout << "Merge event: fd= " << fd
              << " old: " << events_to_str(old_events)
              << " new: " << events_to_str(new_events) << std::endl;

    handlers[id{fd, new_events}] = handler;

    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = old_events | new_events;

    if (epoll_ctl(epoll.get_fd(), EPOLL_CTL_MOD, ev.data.fd, &ev) < 0) {
        perror("Failed to merge event");
    }
}

void event_queue::reset_to_events(std::function<void(struct epoll_event&)> handler, int fd, uint32_t new_events) {
    std::cout << "Reset event: fd= " << fd
              << " new: " << events_to_str(new_events) << std::endl;

    handlers[id{fd, new_events}] = handler;

    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = new_events;

    if (epoll_ctl(epoll.get_fd(), EPOLL_CTL_MOD, ev.data.fd, &ev) < 0) {
        perror("Failed to reset event");
    }
}

void event_queue::delete_events_of_fd(int fd) {
    std::cout << "Delete event: fd= " << fd << std::endl;

    auto it = handlers.find(id{fd, EPOLLIN});
    if (it != handlers.end()) handlers.erase(it);
    it = handlers.find(id{fd, EPOLLOUT});
    if (it != handlers.end()) handlers.erase(it);

    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = NULL;

    // legacy non-NULL ev param with EPOLL_CTL_DEL for kernel before 2.6.9
    if (epoll_ctl(epoll.get_fd(), EPOLL_CTL_DEL, ev.data.fd, &ev) < 0) {
        perror("Failed to delete event");
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
    std::cout << "\nHandle new events: " << amount << std::endl;
    invalid_events.clear();

    for (int i = 0; i < amount; ++i) {
        std::cout << "fd: " << events_list[i].data.fd << " flags: " << events_to_str(events_list[i].events) << std::endl;

        handle_io_events(events_list[i], EPOLLOUT);
        handle_io_events(events_list[i], EPOLLIN);
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

std::ostream& operator<<(std::ostream& os, const event_queue::id& rhs) {
    os << rhs.fd << " " << events_to_str(rhs.events);
    return os;
}

void event_queue::handle_io_events(struct epoll_event& ev, uint32_t events) {
    if (ev.events & events && !invalid_events.count(id{ev.data.fd, events})) {
        id handler_id{ev.data.fd, events};
        if (handlers.count(handler_id)) {
            std::function<void(struct epoll_event&)> handler = handlers[handler_id];
            handler(ev);
        } else {
            std::cout << "INVALID {" <<  events_to_str(events) << "} EVENT" << std::endl;
            for (auto& it : handlers) {
                std::cout << it.first  << std::endl;
            }
            exit();
        }
    }
}
