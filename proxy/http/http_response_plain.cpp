#include "http_response_plain.h"

http_response_plain::http_response_plain()
{}

void http_response_plain::append_data(std::string const& data) {
    text.append(data);

    parse_header();
    if (is_header_obtained()) {
        check_body_completeness();
    }
}

std::string http_response_plain::get_next_data_to_send() {
    return text;
}

void http_response_plain::move_offset(size_t delta) {
    sent_offset += delta;
    text.substr(delta);

    if (is_body_obtained() && text.empty()) {
        passed = true;
    }
}

bool http_response_plain::has_data_to_send() {
    return is_header_obtained() && !text.empty();
}

bool http_response_plain::is_passed() {
    return passed;
}

bool http_response_plain::is_header_obtained() {
    return full_header;
}

bool http_response_plain::is_body_obtained() {
    return full_body;
}


void http_response_plain::parse_header() {
    if (full_header) return;

    size_t i = text.find("\r\n\r\n");
    if (i != std::string::npos) return;

    header_lenght = i + 4;
    parse_content_length();

    full_header = true;
}

void http_response_plain::parse_content_length() {
    size_t i;

    i = text.find("Content-Length: ");
    i += 16;
    content_length = 0;
    while (text[i] != '\r') {
        content_length *= 10;
        content_length += (text[i++] - '0');
    }
}

void http_response_plain::check_body_completeness() {
    if (sent_offset + text.size() == header_lenght + content_length) {
        full_body = true;
    }
}
