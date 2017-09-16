#pragma once

#include <string>

struct http_response_plain {
    http_response_plain();

    void append_data(std::string const& data);
    std::string get_next_data_to_send();
    void move_offset(size_t delta);

    bool has_data_to_send();
    bool is_passed();

private:
    // Contains both header and body
    std::string text;
    // Offset of this plain http response already sent to the client
    size_t sent_offset;

    size_t header_lenght, content_length;

    bool full_header, full_body;
    bool passed;

    bool is_header_obtained();
    bool is_body_obtained();

    void parse_header();
    void parse_content_length();
    void check_body_completeness();
};
