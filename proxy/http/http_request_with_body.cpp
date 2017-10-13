#include "http_request_with_body.h"

http_request_with_body::http_request_with_body(int fd):
    http_request(fd)
{}

void http_request_with_body::append_data(std::string const& data) {
    text.append(data);

    parse_header();
    if (is_header_obtained()) {
        check_body_completeness();
    }
}

std::string http_request_with_body::get_next_data_to_send() const {
    return text;
}

void http_request_with_body::move_offset(size_t delta) {
    assert(delta <= text.size());
    text.erase(0, delta);

    if (is_header_obtained() && text.empty()) {
        passed = true;
    }
}

bool http_request_with_body::has_data_to_send() const noexcept {
    assert(!bad_request);
    return is_header_obtained() && !text.empty();
}

void http_request_with_body::parse_header() noexcept {
    if (full_header) return;

    size_t i = text.find("\r\n\r\n");
    if (i == std::string::npos) return;

    header_lenght = i + 4;
    bad_request = !is_valid_host();
    ensure_relative_url();

    try {
        parse_content_length();
    } catch (...) {
        throw std::runtime_error("POST without content-length are not implemented");
    }

    full_header = true;
}

bool http_request_with_body::is_obtained() const noexcept {
    return is_header_obtained();
}

void http_request_with_body::parse_content_length() noexcept {
    size_t i;

    i = text.find("Content-Length: ");
    i += 16;
    content_length = 0;
    while (text[i] != '\r') {
        content_length *= 10;
        content_length += (text[i++] - '0');
    }
}

void http_request_with_body::check_body_completeness() noexcept {
    if (full_body) return;

    if (text.size() == header_lenght + content_length) {
        full_body = true;
    }
}
