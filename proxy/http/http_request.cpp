#include "http_request.h"

http_request::http_request(int fd):
    text(),
    full_header(false),
    full_body(false),
    passed(false),
    bad_request(false),
    client_fd(fd),
    host()

{}

bool http_request::is_obtained() const noexcept {
    return is_header_obtained() && is_body_obtained();
}

bool http_request::is_passed() const noexcept {
    return passed;
}

bool http_request::is_bad_request() const noexcept {
    return bad_request;
}

// protected
bool http_request::is_header_obtained() const noexcept {
    return full_header;
}

bool http_request::is_body_obtained() const noexcept {
    return full_body;
}

bool http_request::is_valid_host() {
    size_t i = text.find("Host:");
    if (i == std::string::npos) {
        std::cout << "Bad request! No host provided!" << std::endl;
        return false;
    }

    i += 6;
    size_t j = text.find("\r\n", i);
    host = text.substr(i, j - i);
    std::cout << "Request host: " << host << std::endl;
    return true;
}

void http_request::ensure_relative_url() {
    size_t i = text.find(" ");
    size_t j = text.find(" ", i + 1);

    std::string prefix = text.substr(0, i + 1);
    std::string request_url = text.substr(i + 1, j - i - 1);
    std::string suffix = text.substr(j);

    i = request_url.find(host);
    std::string url = request_url;
    if (i != std::string::npos) {
        url = request_url.substr(i + host.size());
    } else {
        std::cout << "INFO: Already relative url" << std::endl;
    }

    text = prefix + url + suffix;
}

// client interface

std::string http_request::get_host() const noexcept {
    return host;
}

int http_request::get_client_fd()  const noexcept {
    return client_fd;
}

//void http_request::set_server_addr(sockaddr addr) {
//    server_addr = addr;
//}

//sockaddr http_request::get_server_addr() const noexcept {
//    return server_addr;
//}
