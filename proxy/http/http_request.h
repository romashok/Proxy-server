#pragma once

#include <string>
#include <iostream>
#include <cassert>

#include "../socket_util.h"
#include "../host_resolver.h"


struct http_request {
    http_request(int fd);

    virtual void append_data(std::string const& data)=0;
    virtual std::string get_next_data_to_send() const =0;
    virtual void move_offset(size_t delta)=0;

    virtual bool has_data_to_send() const noexcept =0 ;
    virtual bool is_obtained() const noexcept;
    virtual bool is_passed() const noexcept;
    virtual bool is_bad_request() const noexcept;

    virtual ~http_request()=default;

    //
    friend class host_resolver;
    std::string get_host() const noexcept;
    int get_client_fd() const noexcept;
    void set_server_addr(struct sockaddr addr);
    sockaddr get_server_addr() const noexcept;


protected:
    // Plaint text of request
    std::string text;

    bool full_header, full_body;
    bool passed, bad_request;

    virtual bool is_header_obtained() const noexcept;
    virtual bool is_body_obtained() const noexcept;

    //

    int client_fd;
    std::string host;
    struct sockaddr server_addr;
    // utils
    bool is_valid_host();
    void ensure_relative_url();
};

