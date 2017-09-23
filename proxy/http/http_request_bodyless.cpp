#include "http_request_bodyless.h"

http_request_bodyless::http_request_bodyless(int fd):
    http_request(fd)
{}

void http_request_bodyless::append_data(std::string const& data) {
    text.append(data);

    parse_header();
}

std::string http_request_bodyless::get_next_data_to_send() const {
    return text;
}

void http_request_bodyless::move_offset(size_t delta) {
    assert(delta <= text.size());
    text.erase(0, delta);

    if (is_header_obtained() && text.empty()) {
        passed = true;
    }
}

bool http_request_bodyless::has_data_to_send() const noexcept {
    assert(!bad_request);
    return is_header_obtained() && !text.empty();
}

void http_request_bodyless::parse_header() noexcept {
    if (full_header) return;

    size_t i = text.find("\r\n\r\n");
    if (i == std::string::npos) return;

    bad_request = !is_valid_host();
    ensure_relative_url();

    full_header = true;
}

bool http_request_bodyless::is_obtained() const noexcept {
    return is_header_obtained();
}
