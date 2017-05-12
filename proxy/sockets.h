#pragma once

struct file_descriptor_t {
    file_descriptor_t& operator=(const file_descriptor_t&)=delete;
    file_descriptor_t(const file_descriptor_t&)=delete;

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

    socket_t()=default;
    socket_t(int fd);

    std::string read(size_t buffer_size);
    size_t write(std::string const& msg);

    ~socket_t();
};
