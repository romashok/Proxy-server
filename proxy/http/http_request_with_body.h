#pragma once

#include "http_request.h"

struct http_request;

struct http_request_with_body : http_request {
    http_request_with_body(int fd);

    void append_data(std::string const& data);
    std::string get_next_data_to_send() const;
    void move_offset(size_t delta);

    bool has_data_to_send() const noexcept ;
    bool is_obtained() const noexcept;

    ~http_request_with_body()=default;

private:
    size_t header_lenght, content_length;

    void parse_header() noexcept;
    void parse_content_length() noexcept;
    void check_body_completeness() noexcept;
};
