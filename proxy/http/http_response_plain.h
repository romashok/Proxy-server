#pragma once

#include "http_response.h"

struct http_response_plain : public http_response {
    http_response_plain()=default;

    void append_data(std::string const& data);
    std::string get_next_data_to_send();
    void move_offset(size_t delta);

    bool has_data_to_send();

    ~http_response_plain()=default;
private:
    // Offset of this plain http response already sent to the client
    size_t sent_offset;

    size_t header_lenght, content_length;

    void parse_header();
    void parse_content_length();
    void check_body_completeness();
};
