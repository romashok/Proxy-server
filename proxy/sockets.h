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
