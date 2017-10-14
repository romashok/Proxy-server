#include "http_response.h"

bool http_response::is_obtained() const noexcept {
    return is_header_obtained() && is_body_obtained();
}

bool http_response::is_passed() const noexcept {
    return passed;
}

bool http_response::is_keep_alive() const noexcept {
    return keep_alive;
}

void http_response::finish_connection() noexcept {
    finished = true;
}

bool http_response::is_header_obtained() const noexcept {
    return full_header;
}

bool http_response::is_body_obtained() const noexcept {
    return full_body;
}

void http_response::parse_connection_type() {
    size_t i = text.find("keep-alive");
    if (i != std::string::npos)
        keep_alive = true;
}
