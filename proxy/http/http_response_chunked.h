#pragma once

#include "http_response.h"

struct http_response_chunked : public http_response {
    http_response_chunked()=default;

    void append_data(std::string const& data);
    std::string get_next_data_to_send() const;
    void move_offset(size_t delta);

    bool has_data_to_send() const noexcept;

    ~http_response_chunked()=default;
private:
    std::string current_chunk;

    void parse_header();
    void extract_next_chunk();
    std::pair<int, size_t> parse_chunk_length();
};
