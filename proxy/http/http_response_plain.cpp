#include "http_response_plain.h"

void http_response_plain::append_data(std::string const& data) {
    text.append(data);

    parse_header();
    if (is_header_obtained()) {
        check_body_completeness();
    }
}

std::string http_response_plain::get_next_data_to_send() const {
    return text;
}

void http_response_plain::move_offset(size_t delta) {
    sent_offset += delta;
    text.erase(0, delta);

    if (is_body_obtained() && text.empty()) {
        passed = true;
    }
}

bool http_response_plain::has_data_to_send() const noexcept {
    return is_header_obtained() && !text.empty();
}

void http_response_plain::parse_header() noexcept {
    if (full_header) return;

    size_t i = text.find("\r\n\r\n");
    if (i == std::string::npos) return;

    std::cout << "response plain header: {\n" << text.substr(0, i) << "\n}" << std::endl;

    header_lenght = i + 4;
    parse_content_length();

    full_header = true;
}

void http_response_plain::parse_content_length() noexcept {
    size_t i;

    i = text.find("Content-Length: ");
    i += 16;
    content_length = 0;
    while (text[i] != '\r') {
        content_length *= 10;
        content_length += (text[i++] - '0');
    }
}

void http_response_plain::check_body_completeness() noexcept {
    if (full_body) return;

    if (sent_offset + text.size() == header_lenght + content_length) {
        full_body = true;
    }
}
