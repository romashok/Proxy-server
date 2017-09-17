#include "http_response_chunked.h"

void http_response_chunked::append_data(std::string const& data) {
    text.append(data);

    parse_header();
}

std::string http_response_chunked::get_next_data_to_send() {
    return current_chunk;
}

void http_response_chunked::move_offset(size_t delta) {
    if (delta == current_chunk.size()) {
        if (is_body_obtained()) {
            passed = true;
            return;
        }

        extract_next_chunk();
    } else {
        current_chunk = current_chunk.substr(delta);
    }
}

bool http_response_chunked::has_data_to_send() {
    return is_header_obtained() && !current_chunk.empty();
}

void http_response_chunked::parse_header() {
    if (full_header) return;

    size_t i = text.find("\r\n\r\n");
    if (i != std::string::npos) return;

    current_chunk = text.substr(0, i + 4);

    // todo rewrite with erase
    if (current_chunk.size() < text.size()) {
        text = text.substr(i + 5);
    } else {
        text.clear();
    }

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

