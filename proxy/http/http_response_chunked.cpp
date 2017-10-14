#include "http_response_chunked.h"

void http_response_chunked::append_data(std::string const& data) {
    text.append(data);

    parse_header();
    validate_last_chunk();
}

std::string http_response_chunked::get_next_data_to_send() const {
    return current_chunk;
}

void http_response_chunked::move_offset(size_t delta) {
    if (delta == current_chunk.size()) {
        current_chunk.clear();

        if (text.empty() && is_body_obtained()) {
            passed = true;
            return;
        }
        extract_next_chunk();
    } else {
        current_chunk.erase(0, delta);
    }
}

bool http_response_chunked::has_data_to_send() const noexcept {
    return is_header_obtained() && !current_chunk.empty();
}

void http_response_chunked::parse_header() {
    if (full_header) return;

    size_t i = text.find("\r\n\r\n");
    if (i == std::string::npos) return;

    parse_connection_type();

    current_chunk = text.substr(0, i + 4);
    text.erase(0, current_chunk.size());
//    std::cout << "response chunked header: {\n" << current_chunk << "}" << std::endl;
//    std::cout << "response chunked rest: {\n" << text << "}" << std::endl;
    full_header = true;
}

void http_response_chunked::extract_next_chunk() {
    if (text.empty()) return;

    std::pair<int, size_t> length = parse_chunk_length();
    if (length.first == -1) return;

    if (length.first == 0) {
        full_body = true;
        current_chunk = text;
        text.clear();
    } else {
        // chunk_size \r\n chunk_content \r\n
        size_t chunk_size = length.second + 2 + length.first + 2;
        if (chunk_size <= text.size()) {
            current_chunk = text.substr(0, chunk_size);
            text.erase(0, chunk_size);
        }
    }
}

void http_response_chunked::validate_last_chunk() {
    if (text.empty()) return;

    size_t pos = 0;
    for (;;) {
        size_t i = text.find("\r\n", pos);
        if (i == std::string::npos) return;

        int length_value;
        try {
            length_value = std::stoi(text.substr(pos, (int)i - (int)pos), nullptr, 16);
        } catch (...) {
            std::cerr << "parse error {" << text.substr(pos, (int)i - (int)pos) << "}" << std::endl;
            return;
        }

        pos = i + 2; // skip <chunk size>\r\n
        if (length_value == 0 && text.find("\r\n", pos) != std::string::npos) {
            full_body = true;
            return;
        }

        pos += length_value + 2; // skip <chunk content>\r\n
        if (pos >= text.size()) return;
    }
}

// return length of next chunk or -1 if not enougth data yet
std::pair<int, size_t> http_response_chunked::parse_chunk_length() {
    size_t i = text.find("\r\n");
    if (i == std::string::npos) {
        return std::make_pair(-1, 0);
    }

    int length_value = std::stoi(text.substr(0, i), nullptr, 16);
    size_t length_size = i;
    return std::make_pair(length_value, length_size);
}

