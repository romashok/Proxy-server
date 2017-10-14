#pragma once

#include "http_response.h"

struct http_response_bodyless : public http_response {
    http_response_bodyless()=default;

    void append_data(std::string const& data);
    std::string get_next_data_to_send() const;
    void move_offset(size_t delta);

    bool has_data_to_send() const noexcept;

    ~http_response_bodyless()=default;
private:
    // Offset of this bodyless http response already sent to the client
    size_t sent_offset;

    void parse_header() noexcept;
};
