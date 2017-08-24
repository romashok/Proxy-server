#pragma once
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

struct file_descriptor_t {
    file_descriptor_t& operator=(const file_descriptor_t&)=delete;
    file_descriptor_t(const file_descriptor_t&)=delete;

    file_descriptor_t& operator=(file_descriptor_t&&);
    file_descriptor_t(file_descriptor_t&&);

    file_descriptor_t();
    file_descriptor_t(int fd);

    void set_fd(int new_fd);
    int get_fd() const;

    ~file_descriptor_t();
protected:
    int fd;
};

struct socket_t: public file_descriptor_t {
    socket_t(socket_t const&)=delete;
    socket_t& operator=(socket_t const&)=delete;

    socket_t(socket_t&&)=default;
    socket_t& operator=(socket_t&&)=default;

    socket_t()=default;
    socket_t(int fd);

    static socket_t accept(int fd);

    std::string read(size_t buffer_size);
    size_t write(std::string const& msg);

    ~socket_t();
};

const uint32_t BUFFER_SIZE = 20000;

struct peer_t {
    peer_t(const peer_t&) = delete;
    peer_t& operator=(const peer_t&) = delete;
    peer_t(int fd);

    int get_fd() const;

    std::string& get_buffer();
    void append_to_buffer(std::string& s);
    size_t get_buffer_size() const;
    bool is_full_buffer() const;

    size_t read(size_t buffer_size);
protected:
    socket_t socket;
    std::string buffer;
};

