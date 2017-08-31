#pragma once

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

struct fd_t {
    fd_t& operator=(const fd_t&)=delete;
    fd_t(const fd_t&)=delete;

    fd_t& operator=(fd_t&&);
    fd_t(fd_t&&);

    fd_t();
    fd_t(int fd);

    void set_fd(int new_fd);
    int get_fd() const noexcept;

    ~fd_t();
protected:
    const static int INVALID_FD = -1;
    int fd;
};

struct socket_t: public fd_t {
    socket_t(socket_t const&)=delete;
    socket_t& operator=(socket_t const&)=delete;

    socket_t(socket_t&&)=default;
    socket_t& operator=(socket_t&&)=default;

    socket_t()=default;
    socket_t(int fd);

    std::string read(size_t buffer_size);
    size_t write(std::string const& msg);

    ~socket_t();
};


struct peer_t {
    peer_t(const peer_t&) = delete;
    peer_t& operator=(const peer_t&) = delete;

    peer_t(int fd);

    int get_fd() const noexcept;

    size_t read();
    size_t write();

    std::string& get_buffer();
    void append_to_buffer(std::string& s);
    size_t get_buffer_size() const noexcept;
    bool is_full_buffer() const noexcept;
    bool is_empty_buffer() const noexcept;
protected:
    const static size_t BUFFER_SIZE = 20000;

    socket_t socket;
    std::string buffer;
};

