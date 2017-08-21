#pragma once
#include <string>

#include "sockets.h"

const uint32_t BUFFER_SIZE = 20000;

struct client_t {
    client_t(const client_t&) = delete;
    client_t& operator=(const client_t&) = delete;

    client_t(int fd);

    int get_fd() const;

    std::string& get_buffer();
    void append_to_buffer(std::string& s);
    size_t get_buffer_size() const;
    bool is_full_buffer() const;

    size_t read(size_t buffer_size);
private:
    socket_t socket;
    std::string buffer;
};
