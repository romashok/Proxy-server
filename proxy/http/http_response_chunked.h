#pragma once

#include <string>

struct http_response_chunked {
    http_response_chunked();

    void append_data(std::string const& data);
    std::string get_next_data_to_send();
    void move_offset(size_t delta);

    bool has_data_to_send();
    bool is_passed();

private:
    // Contains header and chunkes
    std::string text;
    std::string current_chunk;

    bool full_header, full_body;
    bool passed;

    bool is_header_obtained();
    bool is_body_obtained();

    void parse_header();
    void extract_next_chunk();
    std::pair<int, size_t> parse_chunk_length();
};
