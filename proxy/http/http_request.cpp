﻿#include <iostream>

#include "http_request.h"

http_request::http_request(int fd):
    state(HEADER),
    client_fd(fd)
{}

request_state http_request::get_state() const noexcept {
    return state;
}

void http_request::set_state(request_state new_state) noexcept {
    state = new_state;
}

void http_request::append_data(std::string const& str) {
    // todo main logic
    std::cout << "http_request::append_data\n";
    switch (state) {
        case HEADER:
        {
            std::string s = header + str;
            size_t i = s.find("\r\n\r\n");
            if (i != std::string::npos) {
                header = s.substr(0, i + 4);
                if (header.size() < s.size()) body = s.substr(i + 5);

                if (!is_valid_host()) return;
                ensure_relative_url();
                if (if_post_or_put()) return;
//                std::cout << "state: " << state << "\nheader: \n" << header << "body:\n" << body << std::endl;
                state = COMPLETED;
            } else {
                header.append(str);
            }
            break;
        }
        case BODY:
        {
            body.append(str);
            if (body.size() == content_length) {
                state = COMPLETED;
            }
            break;
        }
    }

    std::cout << "http_request::append_data::state: " << get_state() << std::endl;
}

std::string http_request::get_raw_text() const noexcept {
    std::string raw_request_text{header};
    if (!body.empty())
        raw_request_text.append(body);

    return raw_request_text;
}

bool http_request::is_valid_host() {
    size_t i = header.find("Host:");
    if (i == std::string::npos) {
        state = BAD_REQUEST;
        std::cout << "Bad request! No host provided!" << std::endl;
        return false;
    }

    i += 6;
    size_t j = header.find("\r\n", i);
    host = header.substr(i, j - i);
    std::cout << "Request host: " << host << std::endl;
    return true;
}

void http_request::ensure_relative_url() {
    std::cout << "http_request::ensure_relative_url" << std::endl;
    size_t i = header.find(" ");
    size_t j = header.find(" ", i + 1);

    std::string prefix = header.substr(0, i + 1);
    std::string request_url = header.substr(i + 1, j - i - 1);
    std::string suffix = header.substr(j);

    i = request_url.find(host);
    if (i != std::string::npos) {
        url = request_url.substr(i + host.size());
    } else {
        std::cout << "INFO: Already relative url" << std::endl;
    }

    header = prefix + url + suffix;
}

bool http_request::if_post_or_put() {
    std::cout << "http_request::if_post_or_put" << std::endl;
    size_t i = header.find("POST"), j = header.find("PUT") ;
    if (i == std::string::npos && j == std::string::npos) return false;
    // todo chanked transfer

    i = header.find("Content-Length: ");
    i += 16;
    content_length = 0;
    while (header[i] != '\r') {
        content_length *= 10;
        content_length += (header[i++] - '0');
    }

    if (body.size() == content_length) {
        state = COMPLETED;
    } else {
        state = BODY;
    }

    return true;
}

std::string http_request::get_host() const noexcept {
    return host;
}

int http_request::get_client_fd()  const noexcept {
    return client_fd;
}

void http_request::set_server_addr(sockaddr addr) {
    server_addr = addr;
}

sockaddr http_request::get_server_addr() const noexcept {
    return server_addr;
}