#pragma once

#include "http_response.h"

struct http_response_closed_connection : public http_response {
    http_response_closed_connection()=default;

    void append_data(std::string const& data);
    std::string get_next_data_to_send() const;
    void move_offset(size_t delta);

    bool has_data_to_send() const noexcept;

    ~http_response_closed_connection()=default;
private:
    // Offset of this closed_connection http response already sent to the client
    size_t sent_offset;

    size_t header_lenght, content_length;

    void parse_header() noexcept;
    void parse_content_length() noexcept;
    void check_body_completeness() noexcept;
};
