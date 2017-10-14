#include "http_response_bodyless.h"

void http_response_bodyless::append_data(std::string const& data) {
    text.append(data);

    parse_header();
}

std::string http_response_bodyless::get_next_data_to_send() const {
    return text;
}

void http_response_bodyless::move_offset(size_t delta) {
    sent_offset += delta;
    text.erase(0, delta);

    if (is_header_obtained() && text.empty()) {
        passed = true;
    }
}

bool http_response_bodyless::has_data_to_send() const noexcept {
    return is_header_obtained() && !text.empty();
}

void http_response_bodyless::parse_header() noexcept {
    if (full_header) return;

    size_t i = text.find("\r\n\r\n");
    if (i == std::string::npos) return;

    parse_connection_type();
    std::cout << "response bodyless header: {\n" << text.substr(0, i) << "\n}" << std::endl;

    full_header = true;
    full_body = true;
}

